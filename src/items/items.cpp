/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2024 OpenTibiaBR <opentibiabr@outlook.com>
 * Repository: https://github.com/opentibiabr/canary
 * License: https://github.com/opentibiabr/canary/blob/main/LICENSE
 * Contributors: https://github.com/opentibiabr/canary/graphs/contributors
 * Website: https://docs.opentibiabr.com/
 */

#include "items/items.hpp"

#include "config/configmanager.hpp"
#include "game/game.hpp"
#include "items/functions/item/item_parse.hpp"
#include "items/weapons/weapons.hpp"
#include "lua/creature/movement.hpp"
#include "utils/pugicast.hpp"
#include "creatures/combat/spells.hpp"
#include "utils/tools.hpp"

#include <appearances.pb.h>

Items::Items() = default;

void Items::clear() {
	items.clear();
	ladders.clear();
	dummys.clear();
	nameToItems.clear();
	g_moveEvents().clear();
	g_weapons().clear(true);
}

using LootTypeNames = phmap::flat_hash_map<std::string, ItemTypes_t>;

LootTypeNames lootTypeNames = {
	{ "armor", ITEM_TYPE_ARMOR },
	{ "amulet", ITEM_TYPE_AMULET },
	{ "boots", ITEM_TYPE_BOOTS },
	{ "container", ITEM_TYPE_CONTAINER },
	{ "decoration", ITEM_TYPE_DECORATION },
	{ "food", ITEM_TYPE_FOOD },
	{ "helmet", ITEM_TYPE_HELMET },
	{ "legs", ITEM_TYPE_LEGS },
	{ "other", ITEM_TYPE_OTHER },
	{ "potion", ITEM_TYPE_POTION },
	{ "ring", ITEM_TYPE_RING },
	{ "rune", ITEM_TYPE_RUNE },
	{ "shield", ITEM_TYPE_SHIELD },
	{ "tools", ITEM_TYPE_TOOLS },
	{ "valuable", ITEM_TYPE_VALUABLE },
	{ "ammo", ITEM_TYPE_AMMO },
	{ "axe", ITEM_TYPE_AXE },
	{ "club", ITEM_TYPE_CLUB },
	{ "distance", ITEM_TYPE_DISTANCE },
	{ "sword", ITEM_TYPE_SWORD },
	{ "wand", ITEM_TYPE_WAND },
	{ "creatureproduct", ITEM_TYPE_CREATUREPRODUCT },
	{ "retrieve", ITEM_TYPE_RETRIEVE },
	{ "gold", ITEM_TYPE_GOLD },
	{ "unassigned", ITEM_TYPE_UNASSIGNED },
};

ItemTypes_t Items::getLootType(const std::string &strValue) const {
	const auto lootType = lootTypeNames.find(strValue);
	if (lootType != lootTypeNames.end()) {
		return lootType->second;
	}
	return ITEM_TYPE_NONE;
}

std::string Items::getAugmentNameByType(Augment_t augmentType) {
	std::string augmentTypeName = magic_enum::enum_name(augmentType).data();
	augmentTypeName = toStartCaseWithSpace(augmentTypeName);
	if (!isAugmentWithoutValueDescription(augmentType)) {
		toLowerCaseString(augmentTypeName);
	}
	return augmentTypeName;
}

std::string ItemType::parseAugmentDescription(bool inspect /*= false*/) const {
	if (augments.empty()) {
		return "";
	}

	std::vector<std::string> descriptions;
	for (const auto &augment : augments) {
		descriptions.push_back(getFormattedAugmentDescription(augment));
	}

	if (inspect) {
		return fmt::format("{}.", fmt::join(descriptions.begin(), descriptions.end(), ", "));
	} else {
		return fmt::format("\nAugments: ({}).", fmt::join(descriptions.begin(), descriptions.end(), ", "));
	}
}

std::string ItemType::getFormattedAugmentDescription(const std::shared_ptr<AugmentInfo> &augmentInfo) const {
	const auto augmentName = Items::getAugmentNameByType(augmentInfo->type);
	std::string augmentSpellNameCapitalized = augmentInfo->spellName;
	capitalizeWordsIgnoringString(augmentSpellNameCapitalized, " of ");

	char signal = augmentInfo->value > 0 ? '-' : '+';

	if (Items::isAugmentWithoutValueDescription(augmentInfo->type)) {
		return fmt::format("{} -> {}", augmentSpellNameCapitalized, augmentName);
	} else if (augmentInfo->type == Augment_t::Cooldown) {
		return fmt::format("{} -> {}{}s {}", augmentSpellNameCapitalized, signal, augmentInfo->value / 1000, augmentName);
	} else if (augmentInfo->type == Augment_t::Base) {
		const auto &spell = g_spells().getSpellByName(augmentInfo->spellName);
		return fmt::format("{} -> {:+}% {} {}", augmentSpellNameCapitalized, augmentInfo->value, augmentName, spell->getGroup() == SPELLGROUP_HEALING ? "healing" : "damage");
	}

	return fmt::format("{} -> {:+}% {}", augmentSpellNameCapitalized, augmentInfo->value, augmentName);
}

void ItemType::addAugment(std::string spellName, Augment_t augmentType, int32_t value) {
	auto augmentInfo = std::make_shared<AugmentInfo>(spellName, augmentType, value);
	augments.emplace_back(augmentInfo);
}

void ItemType::setImbuementType(ImbuementTypes_t imbuementType, uint16_t slotMaxTier) {
	imbuementTypes[imbuementType] = std::min<uint16_t>(IMBUEMENT_MAX_TIER, slotMaxTier);
}

bool ItemType::triggerExhaustion() const {
	return isRune() || (type == ITEM_TYPE_POTION && !m_isMagicShieldPotion);
}

bool Items::reload() {
	clear();
	if (g_configManager().getBoolean(LOAD_ITEMS_FROM_SPR_DAT)) {
		loadFromDat();
	} else {
		loadFromProtobuf();
	}

	if (!loadFromXml()) {
		return false;
	}

	return true;
}

void Items::loadFromProtobuf() {
	using namespace Canary::protobuf::appearances;

	bool supportAnimation = g_configManager().getBoolean(OLD_PROTOCOL);
	for (uint32_t it = 0; it < g_game().m_appearancesPtr->object_size(); ++it) {
		Appearance object = g_game().m_appearancesPtr->object(it);

		// This scenario should never happen but on custom assets this can break the loader.
		if (!object.has_flags()) {
			g_logger().warn("[Items::loadFromProtobuf] - Item with id '{}' is invalid and was ignored.", object.id());
			continue;
		}

		if (object.id() >= items.size()) {
			items.resize(object.id() + 1);
		}

		if (!object.has_id()) {
			continue;
		}

		ItemType &iType = items[object.id()];
		if (object.flags().container()) {
			iType.type = ITEM_TYPE_CONTAINER;
			iType.group = ITEM_GROUP_CONTAINER;
		} else if (object.flags().has_bank()) {
			iType.group = ITEM_GROUP_GROUND;
		} else if (object.flags().liquidcontainer()) {
			iType.group = ITEM_GROUP_FLUID;
		} else if (object.flags().liquidpool()) {
			iType.group = ITEM_GROUP_SPLASH;
		}

		// This attribute is only used on 10x protocol, so we should not waste our time iterating it when it's disabled.
		if (supportAnimation) {
			for (uint32_t frame_it = 0; frame_it < object.frame_group_size(); ++frame_it) {
				const FrameGroup &objectFrame = object.frame_group(frame_it);
				if (!objectFrame.has_sprite_info()) {
					continue;
				}

				if (!objectFrame.sprite_info().has_animation()) {
					continue;
				}

				if (objectFrame.sprite_info().animation().random_start_phase()) {
					iType.animationType = ANIMATION_RANDOM;
				} else {
					iType.animationType = ANIMATION_DESYNC;
				}
			}
		}

		if (object.flags().clip()) {
			iType.alwaysOnTopOrder = 1;
		} else if (object.flags().top()) {
			iType.alwaysOnTopOrder = 3;
		} else if (object.flags().bottom()) {
			iType.alwaysOnTopOrder = 2;
		}

		if (object.flags().has_clothes()) {
			iType.slotPosition |= static_cast<SlotPositionBits>(1 << (object.flags().clothes().slot() - 1));
		}

		if (object.flags().has_market()) {
			iType.type = static_cast<ItemTypes_t>(object.flags().market().category());
		}

		iType.name = object.name();
		iType.description = object.description();

		iType.upgradeClassification = object.flags().has_upgradeclassification() ? static_cast<uint8_t>(object.flags().upgradeclassification().upgrade_classification()) : 0;
		iType.lightLevel = object.flags().has_light() ? static_cast<uint8_t>(object.flags().light().brightness()) : 0;
		iType.lightColor = object.flags().has_light() ? static_cast<uint8_t>(object.flags().light().color()) : 0;

		iType.id = static_cast<uint16_t>(object.id());
		iType.speed = object.flags().has_bank() ? static_cast<uint16_t>(object.flags().bank().waypoints()) : 0;
		iType.wareId = object.flags().has_market() ? static_cast<uint16_t>(object.flags().market().trade_as_object_id()) : 0;

		iType.isCorpse = object.flags().corpse() || object.flags().player_corpse();
		iType.forceUse = object.flags().forceuse();
		iType.hasHeight = object.flags().has_height();
		iType.blockSolid = object.flags().unpass();
		iType.blockProjectile = object.flags().unsight();
		iType.blockPathFind = object.flags().avoid();
		iType.pickupable = object.flags().take();
		iType.rotatable = object.flags().rotate();
		iType.wrapContainer = object.flags().wrap() || object.flags().unwrap();
		if (iType.wrapContainer) {
			iType.wrapableTo = ITEM_DECORATION_KIT;
			iType.wrapable = true;
		}
		iType.multiUse = object.flags().multiuse();
		iType.movable = object.flags().unmove() == false;
		iType.canReadText = (object.flags().has_lenshelp() && object.flags().lenshelp().id() == 1112) || (object.flags().has_write() && object.flags().write().max_text_length() != 0) || (object.flags().has_write_once() && object.flags().write_once().max_text_length_once() != 0);
		iType.canReadText = object.flags().has_write() || object.flags().has_write_once();
		iType.isVertical = object.flags().has_hook() && object.flags().hook().direction() == HOOK_TYPE_SOUTH;
		iType.isHorizontal = object.flags().has_hook() && object.flags().hook().direction() == HOOK_TYPE_EAST;
		iType.isHangable = object.flags().hang();
		iType.lookThrough = object.flags().ignore_look();
		iType.stackable = object.flags().cumulative();
		iType.isPodium = object.flags().show_off_socket();
		iType.wearOut = object.flags().wearout();
		iType.clockExpire = object.flags().clockexpire();
		iType.expire = object.flags().expire();
		iType.expireStop = object.flags().expirestop();
		iType.isWrapKit = object.flags().wrapkit();

		if (!iType.name.empty()) {
			nameToItems.insert({ asLowerCaseString(iType.name), iType.id });
		}
	}

	items.shrink_to_fit();
}

bool Items::loadFromXml() {
	pugi::xml_document doc;
	auto folder = g_configManager().getString(CORE_DIRECTORY) + "/items/items.xml";
	pugi::xml_parse_result result = doc.load_file(folder.c_str());
	if (!result) {
		printXMLError(__FUNCTION__, folder, result);
		return false;
	}

	for (const auto itemNode : doc.child("items").children()) {
		if (auto idAttribute = itemNode.attribute("id")) {
			parseItemNode(itemNode, pugi::cast<uint16_t>(idAttribute.value()));
			continue;
		}

		auto fromIdAttribute = itemNode.attribute("fromid");
		if (!fromIdAttribute) {
			g_logger().warn("[Items::loadFromXml] - No item id found, use id or fromid");
			continue;
		}

		auto toIdAttribute = itemNode.attribute("toid");
		if (!toIdAttribute) {
			g_logger().warn("[Items::loadFromXml] - "
			                "tag fromid: {} without toid",
			                fromIdAttribute.value());
			continue;
		}

		auto id = pugi::cast<uint16_t>(fromIdAttribute.value());
		const auto toId = pugi::cast<uint16_t>(toIdAttribute.value());
		while (id <= toId) {
			parseItemNode(itemNode, id++);
		}
	}
	return true;
}

bool Items::loadFromDat() {
	auto file = g_configManager().getString(CORE_DIRECTORY) + "/items/Tibia.dat";
	std::filesystem::path filePath(file);

	if (!std::filesystem::exists(filePath)) {
		g_logger().warn("[Items::loadFromDat] - File not found.");
		return false;
	}

	std::ifstream fileStream(filePath, std::ios::binary);
	if (!fileStream.is_open()) {
		g_logger().warn("[Items::loadFromDat] - Failed to open file.");
		return false;
	}

	std::streamsize fileSize = std::filesystem::file_size(filePath);
	std::vector<char> buffer(fileSize);
	if (!fileStream.read(buffer.data(), fileSize)) {
		g_logger().warn("[Items::loadFromDat] - Failed to read file.");
		fileStream.close();
		return false;
	}
	fileStream.close();

	PropStream props;
	props.init(buffer.data(), fileSize);

	auto signature = props.read<uint32_t>();
	auto objectCount = props.read<uint16_t>();
	auto outfitCount = props.read<uint16_t>();
	auto effectCount = props.read<uint16_t>();
	auto missileCount = props.read<uint16_t>();

	g_logger().info("Dat signature: {}", signature);
	g_logger().info("Dat objectCount: {}", objectCount);
	g_logger().info("Dat outfitCount: {}", outfitCount);
	g_logger().info("Dat effectCount: {}", effectCount);
	g_logger().info("Dat missileCount: {}", missileCount);

	// Exemplo: IDs de itens começam em 100 e vão até (objectCount - 1),
	// mas isto assume que objectCount já é o "limite" + 100.
	// Em muitas bases, faz-se algo como (firstId + objectCount - 1)
	// para o final, mas manteremos conforme o seu uso original.
	uint16_t firstId = 100;

	uint16_t lastItemId = firstId + objectCount - 1;

	// Loop para ler cada ITEM (não confundir com outfits/effects/missiles)
	for (uint16_t id = firstId; id <= lastItemId; ++id) {

		/*  for (uint16_t id = firstId; id < objectCount; ++id) { */
		if (id >= items.size()) {
			items.resize(id + 1);
		}
		ItemType &item = items[id];
		item.id = id;

		// Em vez de (for i = 0; i < DatAttrDefault; ++i),
		// vamos ler até achar 0xFF (fim de atributos).
		while (true) {
			uint8_t attr = props.read<uint8_t>();

			// Se achar 0xFF, saímos do loop de atributos
			if (attr == 0xFF) {
				break;
			}

			switch (attr) {
				case DatAttrGround:
					item.group = ITEM_GROUP_GROUND;
					item.speed = props.read<uint16_t>();
					break;

				case DatAttrClip:
					item.alwaysOnTopOrder = 1;
					break;

				case DatAttrTop:
					item.alwaysOnTopOrder = 3;
					break;

				case DatAttrBottom:
					item.alwaysOnTopOrder = 2;
					break;

				case DatAttrContainer:
					item.group = ITEM_GROUP_CONTAINER;
					item.type = ITEM_TYPE_CONTAINER;
					break;

				case DatAttrStackable: {
					item.stackable = true;
					break;
				}

				case DatAttrUsable:
					break;

				case DatAttrForceUse:
					item.forceUse = true;
					break;

				case DatAttrMultiUse:
					item.multiUse = true;
					break;

				case DatAttrWriteable:
					item.canReadText = true;
					item.maxTextLen = props.read<uint16_t>();
				case DatAttrWriteableOnce:
					item.canReadText = true;
					item.maxTextLen = props.read<uint16_t>();
					break;
				case DatAttrLiquidPool:
					item.group = ITEM_GROUP_SPLASH;
					break;

				case DatAttrLiquidContainer:
					item.group = ITEM_GROUP_FLUID;
					break;

				case DatAttrImpassable:
					item.blockSolid = true;
					break;

				case DatAttrUnmovable:
					item.movable = false;
					break;

				case DatAttrBlocksSight:
					item.blockProjectile = true;
					break;

				case DatAttrBlocksPathfinding:
					item.blockPathFind = true;
					break;

				case DatAttrNoMovementAnimation:
					break;

				case DatAttrPickupable:
					item.pickupable = true;
					break;

				case DatAttrHangable:
					item.isHangable = true;
					break;

				case DatAttrHooksSouth:
					item.isVertical = true;
					break;

				case DatAttrHooksEast:
					item.isHorizontal = true;
					break;

				case DatAttrRotateable:
					item.rotatable = true;
					break;

				case DatAttrLightSource:
					item.lightLevel = props.read<uint16_t>();
					item.lightColor = props.read<uint16_t>();
					break;

				case DatAttrAlwaysSeen:
					break;

				case DatAttrTranslucent:
					break;

				case DatAttrDisplaced:
					props.skip(2); // unknown u16 flag
					props.skip(2); // unknown u16 flag
					break;

				case DatAttrElevated:
					item.hasHeight = true;
					props.read<uint16_t>();
					break;

				case DatAttrAlwaysAnimated:
					break;

				case DatAttrMinimapColor:
					props.read<uint16_t>();
					break;

				case DatAttrFullTile:
					item.group = ITEM_GROUP_GROUND;
					break;

				case DatAttrHelpInfo: {
					uint16_t opt = props.read<uint16_t>();
					if (opt == 1112) {
						item.canReadText = true;
					}
					break;
				}

				case DatAttrLookthrough:
					item.lookThrough = true;
					break;

				case DatAttrClothes:
					props.read<uint16_t>();
					break;

				case DatAttrMarket: {
					auto category = props.read<uint16_t>();
					auto tradeAsObjectId = props.read<uint16_t>();
					auto showAsObjectId = props.read<uint16_t>();
					auto name = props.readString();
					auto vocation = props.read<uint16_t>();
					auto minimumLevel = props.read<uint16_t>();
					break;
				}

				case DatAttrDefaultAction:
					props.read<uint16_t>();
					break;

				case DatAttrWrappable:
					item.wrapContainer = true;
					item.wrapable = true;
					item.wrapableTo = ITEM_DECORATION_KIT;
					break;
				case DatAttrUnWrappable:
					item.wrapContainer = true;
					break;
				case DatAttrTopEffect: {
					break;
				}

				case DatAttrNpcSaleData: {
					break;
				}
				case DatAttrChangedToExpire: {
					item.wareId = props.read<uint16_t>();
					break;
				}
				case DatAttrCorpse: {
					item.isCorpse = true;
					break;
				}
				case DatAttrPlayerCorpse: {
					item.isCorpse = true;
					break;
				}
				case DatAttrCyclopediaItem: {
					props.read<uint16_t>(); // CyclopediaType
					break;
				}
				case DatAttrAmmo: {
					break;
				}
				case DatAttrShowOffSocket: {
					item.isPodium = true;
					break;
				}
				case DatAttrReportable: {
					break;
				}
				case DatAttrUpgradeClassification: {
					item.upgradeClassification = props.read<uint16_t>();
					break;
				}
				case DatAttrWearout: {
					item.wearOut = true;
					break;
				}
				case DatAttrClockExpire: {
					item.clockExpire = true;
					break;
				}
				case DatAttrExpire: {
					item.expire = true;
					break;
				}
				case DatAttrExpireStop: {
					item.expireStop = true;
					break;
				}

				default: {
					break;
				}
			}
		}

		// Leitura das dimensões e sprite data
		uint8_t width = props.read<uint8_t>();
		uint8_t height = props.read<uint8_t>();
		if (width > 1 || height > 1) {
			props.skip(1); // "exact size"
		}

		uint8_t layers = props.read<uint8_t>();
		uint8_t patternX = props.read<uint8_t>();
		uint8_t patternY = props.read<uint8_t>();
		uint8_t patternZ = props.read<uint8_t>();
		uint8_t phases = props.read<uint8_t>();

		// Se tiver animação
		if (phases > 1) {
			uint8_t animMode = props.read<uint8_t>();
			item.animationType = (animMode == 1) ? ANIMATION_DESYNC : ANIMATION_RANDOM;

			// pular 5 bytes (frameDurations, loopCount etc)
			props.skip(5);

			// pular 8 bytes para cada phase
			for (int16_t i = 0; i < phases; i++) {
				props.skip(8);
			}
		}

		// Finalmente, pular a quantidade de sprites (4 bytes por sprite)
		uint32_t totalSprites = width * height * layers * patternX * patternY * patternZ * phases;
		props.skip(4 * totalSprites);
	}

	std::vector<uint16_t> outfits;
	outfits.reserve(outfitCount);
	for (uint16_t i = 0; i < outfitCount; ++i) {
		outfits.push_back(i);
	}

	std::vector<uint16_t> magicEffects;
	magicEffects.reserve(effectCount);
	for (uint16_t i = 0; i < effectCount; ++i) {
		magicEffects.push_back(i);
	}

	std::vector<uint16_t> distanceEffects;
	distanceEffects.reserve(missileCount);
	for (uint16_t i = 0; i < missileCount; ++i) {
		distanceEffects.push_back(i);
	}

	g_logger().debug("Loaded Items: {}, Outfits: {}, MagicEffects: {}, DistanceEffects: {}", items.size(), outfits.size(), magicEffects.size(), distanceEffects.size());

	g_game().setLookTypes(outfits);
	g_game().setMagicEffectTypes(magicEffects);
	g_game().setDistanceEffectTypes(distanceEffects);

	return true;
}

void Items::buildInventoryList() {
	inventory.reserve(items.size());
	for (const auto &type : items) {
		if (type.weaponType != WEAPON_NONE || type.ammoType != AMMO_NONE || type.attack != 0 || type.defense != 0 || type.extraDefense != 0 || type.armor != 0 || type.slotPosition & SLOTP_NECKLACE || type.slotPosition & SLOTP_RING || type.slotPosition & SLOTP_AMMO || type.slotPosition & SLOTP_FEET || type.slotPosition & SLOTP_HEAD || type.slotPosition & SLOTP_ARMOR || type.slotPosition & SLOTP_LEGS) {
			inventory.push_back(type.id);
		}
	}
	inventory.shrink_to_fit();
	std::ranges::sort(inventory);
}

void Items::parseItemNode(const pugi::xml_node &itemNode, uint16_t id) {
	if (id >= items.size()) {
		items.resize(id + 1);
	}
	ItemType &itemType = getItemType(id);
	// Ids 0-100 are used for fluids in the XML
	if (id >= 100 && (itemType.id == 0 && (itemType.name.empty() || itemType.name == asLowerCaseString("reserved sprite")))) {
		return;
	}
	itemType.id = id;

	if (itemType.loaded) {
		g_logger().warn("[Items::parseItemNode] - Duplicate item with id: {}", id);
		return;
	}

	if (const std::string xmlName = itemNode.attribute("name").as_string();
	    !xmlName.empty() && itemType.name != xmlName) {
		if (!itemType.name.empty()) {
			if (const auto it = std::ranges::find_if(nameToItems, [id](const auto nameMapIt) {
					return nameMapIt.second == id;
				});
			    it != nameToItems.end()) {
				nameToItems.erase(it);
			}
		}

		itemType.name = xmlName;
		nameToItems.insert({ asLowerCaseString(itemType.name), id });
	}

	itemType.loaded = true;
	const pugi::xml_attribute articleAttribute = itemNode.attribute("article");
	if (articleAttribute) {
		itemType.article = articleAttribute.as_string();
	}

	const pugi::xml_attribute pluralAttribute = itemNode.attribute("plural");
	if (pluralAttribute) {
		itemType.pluralName = pluralAttribute.as_string();
	}

	for (const auto &attributeNode : itemNode.children()) {
		const pugi::xml_attribute keyAttribute = attributeNode.attribute("key");
		if (!keyAttribute) {
			continue;
		}

		const pugi::xml_attribute valueAttribute = attributeNode.attribute("value");
		if (!valueAttribute) {
			continue;
		}

		const std::string tmpStrValue = asLowerCaseString(keyAttribute.as_string());
		auto parseAttribute = ItemParseAttributesMap.find(tmpStrValue);
		if (parseAttribute != ItemParseAttributesMap.end()) {
			ItemParse::initParse(tmpStrValue, attributeNode, valueAttribute, itemType);
		} else {
			g_logger().warn("[Items::parseItemNode] - Unknown key value: {}", keyAttribute.as_string());
		}
	}

	// Check bed items
	if ((itemType.transformToFree != 0 || itemType.transformToOnUse[PLAYERSEX_FEMALE] != 0 || itemType.transformToOnUse[PLAYERSEX_MALE] != 0) && itemType.type != ITEM_TYPE_BED) {
		g_logger().warn("[Items::parseItemNode] - Item {} is not set as a bed-type", itemType.id);
	}
}

ItemType &Items::getItemType(size_t id) {
	if (id < items.size()) {
		return items[id];
	}
	return items.front();
}

const ItemType &Items::getItemType(size_t id) const {
	if (id < items.size()) {
		return items[id];
	}
	return items.front();
}

uint16_t Items::getItemIdByName(const std::string &name) {
	const auto result = nameToItems.find(asLowerCaseString(name));

	if (result == nameToItems.end()) {
		return 0;
	}

	return result->second;
}

bool Items::hasItemType(size_t hasId) const {
	if (hasId < items.size()) {
		return true;
	}
	return false;
}

uint32_t Abilities::getHealthGain() const {
	return healthGain * g_configManager().getFloat(RATE_HEALTH_REGEN);
}

uint32_t Abilities::getHealthTicks() const {
	return healthTicks / g_configManager().getFloat(RATE_HEALTH_REGEN_SPEED);
}

uint32_t Abilities::getManaGain() const {
	return manaGain * g_configManager().getFloat(RATE_MANA_REGEN);
}

uint32_t Abilities::getManaTicks() const {
	return manaTicks / g_configManager().getFloat(RATE_MANA_REGEN_SPEED);
}
