/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2024 OpenTibiaBR
 * Repository: https://github.com/opentibiabr/canary
 */

#include "creatures/players/missions/mission_manager.hpp"
#include "creatures/players/player.hpp"
#include "items/containers/inbox/inbox.hpp"
#include "creatures/monsters/monster.hpp"
#include "creatures/monsters/monsters.hpp"
#include "lua/functions/events/action_functions.hpp"
#include "lua/creature/actions.hpp"
#include "game/game.hpp"
#include "config/configmanager.hpp"
#include "lib/di/container.hpp"
#include "utils/pugicast.hpp"
#include "utils/tools.hpp"
#include "kv/kv.hpp"

MissionManager &MissionManager::getInstance() {
	static MissionManager instance;
	return instance;
}

bool MissionManager::loadFromXML() {
	missions.clear();

	const std::string directory = g_configManager().getString(CORE_DIRECTORY) + "/XML/missions/";

	std::vector<std::string> fileList;
	if (!listDirectoryFiles(directory, fileList, ".xml")) {
		g_logger().warn("[MissionManager] Could not open missions directory '{}'", directory);
		return false;
	}

	if (fileList.empty()) {
		g_logger().warn("[MissionManager] No mission files found in '{}'", directory);
		return false;
	}

	for (const auto &file : fileList) {
		loadMissionFile(directory + file);
	}

	g_logger().info("[MissionManager] Loaded {} mission(s).", missions.size());
	return true;
}

void MissionManager::loadMissionFile(const std::string &filepath) {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filepath.c_str());
	if (!result) {
		printXMLError(__FUNCTION__, filepath, result);
		return;
	}

	const auto root = doc.child("mission");
	if (!root) {
		g_logger().warn("[MissionManager] Missing <mission> root in '{}'", filepath);
		return;
	}

	Mission mission;
	pugi::xml_attribute attr;

	if (!(attr = root.attribute("id"))) {
		g_logger().warn("[MissionManager] Missing id for mission in '{}'", filepath);
		return;
	}
	mission.id = pugi::cast<uint32_t>(attr.value());

	if (!(attr = root.attribute("name"))) {
		g_logger().warn("[MissionManager] Missing name for mission '{}'", filepath);
		return;
	}
	mission.name = attr.as_string();

	mission.repeatable = std::string(root.attribute("repeatable").as_string()) == "true";
	mission.description = root.child_value("description");

	for (const auto &stageNode : root.child("stages").children("stage")) {
		MissionStage stage;

		if (!(attr = stageNode.attribute("id"))) {
			g_logger().warn("Missing stage ID in mission '{}'", mission.name);
			continue;
		}
		stage.id = pugi::cast<uint32_t>(attr.value());
		stage.name = stageNode.attribute("name").as_string();

		if (const auto reqs = stageNode.child("requirements")) {
			for (const auto &req : reqs.children()) {
				const std::string tag = req.name();
				if (tag == "kv") {
					std::string key = req.attribute("key").as_string();
					std::string value = req.attribute("value").as_string();
					if (!key.empty()) {
						stage.kvRequirements.emplace_back(key, value);
					}
				} else if (tag == "level") {
					stage.requirements["level_min"] = req.attribute("min").as_string();
				} else if (tag == "skill") {
					stage.requirements["skill_name"] = req.attribute("name").as_string();
					stage.requirements["skill_level"] = req.attribute("level").as_string();
				} else if (tag == "item") {
					stage.requirements["item_id"] = req.attribute("id").as_string();
					stage.requirements["item_count"] = req.attribute("count").as_string();
				} else if (tag == "spell") {
					stage.requirements["spell_name"] = req.attribute("name").as_string();
				} else if (tag == "vocation") {
					stage.requirements["vocation_id"] = req.attribute("id").as_string();
				} else if (tag == "party") {
					stage.requirements["party_required"] = req.attribute("required").as_string();
				}
			}
		}

		for (const auto &obj : stageNode.child("objectives").children()) {
			MissionObjective objective;
			const std::string tag = obj.name();

			if (tag == "kill") {
				objective.type = MissionObjectiveType::KILL;
				objective.param = obj.attribute("monster").as_string();
			} else if (tag == "collect") {
				objective.type = MissionObjectiveType::COLLECT;
				objective.param = obj.attribute("itemid").as_string();
			} else if (tag == "investigate" || tag == "sabotage" || tag == "go" || tag == "use") {
				objective.type = (tag == "use") ? MissionObjectiveType::SABOTAGE : MissionObjectiveType::INVESTIGATE;
				objective.param = obj.attribute("itemid").as_string();
				objective.description = obj.attribute("description").as_string();
			} else if (tag == "talk") {
				objective.type = MissionObjectiveType::TALK;
				objective.param = obj.attribute("npc").as_string();
			} else if (tag == "escort") {
				objective.type = MissionObjectiveType::ESCORT;
				objective.param = obj.attribute("npc").as_string();
				objective.description = obj.attribute("position").as_string();
			} else if (tag == "defend") {
				objective.type = MissionObjectiveType::DEFEND;
				objective.param = obj.attribute("npc").as_string();
			} else if (tag == "capture") {
				objective.type = MissionObjectiveType::CAPTURE;
				objective.param = obj.attribute("npc").as_string();
			} else if (tag == "survive") {
				objective.type = MissionObjectiveType::SURVIVE;
				objective.param = obj.attribute("seconds").as_string();
			} else if (tag == "useon") { // ✅ Novo objetivo
				objective.type = MissionObjectiveType::USE_ON;
				objective.itemId = obj.attribute("itemid").as_uint();
				objective.param = obj.attribute("target").as_string(); // Nome da criatura alvo
			} else {
				g_logger().warn("Unknown objective '{}' in mission '{}'", tag, mission.name);
				continue;
			}

			objective.amount = obj.attribute("amount").as_uint(1); // Padrão: 1 se não existir
			objective.description = obj.attribute("description").as_string();
			stage.objectives.emplace_back(std::move(objective));
		}

		for (const auto &reward : stageNode.child("rewards").children()) {
			MissionReward r;
			const std::string tag = reward.name();

			if (tag == "xp") {
				r.type = MissionRewardType::XP;
				r.value = reward.attribute("amount").as_uint();
			} else if (tag == "item") {
				r.type = MissionRewardType::ITEM;
				r.name = reward.attribute("id").as_string();
				r.value = reward.attribute("count").as_uint(1);
			} else if (tag == "attributePoints") {
				r.type = MissionRewardType::ATTR_POINTS;
				r.value = reward.attribute("amount").as_uint();
			} else if (tag == "skill") {
				r.type = MissionRewardType::SKILL_TRIES;
				r.name = reward.attribute("name").as_string();
				r.value = reward.attribute("tries").as_uint();
			} else if (tag == "perk") {
				r.type = MissionRewardType::PERK;
				r.name = reward.attribute("id").as_string();
			} else if (tag == "kv") {
				r.type = MissionRewardType::KVE;
				r.name = reward.attribute("key").as_string();
				r.extraValue = reward.attribute("value").as_string();
				r.description = reward.attribute("description").as_string();
			} else if (tag == "spell") {
				r.type = MissionRewardType::SPELL;
				r.name = reward.attribute("name").as_string();
			} else {
				g_logger().warn("Unknown reward '{}' in mission '{}'", tag, mission.name);
				continue;
			}

			stage.rewards.emplace_back(std::move(r));
		}

		mission.stages.emplace_back(std::move(stage));
	}

	missions.emplace_back(std::move(mission));
}

const Mission* MissionManager::getMissionById(uint32_t id) const {
	for (const auto &mission : missions) {
		if (mission.id == id) {
			return &mission;
		}
	}
	return nullptr;
}

void MissionManager::setActiveMissionId(Player &player, uint32_t missionId) const {
	player.kv()->scoped("mission")->scoped("active")->set("id", static_cast<int>(missionId));
}

void MissionManager::setActiveMissionStage(Player &player, uint32_t stage) const {
	player.kv()->scoped("mission")->scoped("active")->set("stage", static_cast<int>(stage));
}

uint32_t MissionManager::getActiveMissionId(Player &player) const {
	const auto id = player.kv()->scoped("mission")->scoped("active")->get("id");
	return id ? static_cast<uint32_t>(id->get<int>()) : 0;
}

uint32_t MissionManager::getActiveMissionStageIndex(Player &player) const {
	const auto stage = player.kv()->scoped("mission")->scoped("active")->get("stage");
	return stage ? static_cast<uint32_t>(stage->get<int>()) : 0;
}

std::string MissionManager::getPlayerMissionStatus(Player &player) const {
	const auto idValue = player.kv()->scoped("mission")->scoped("active")->get("id");
	if (!idValue || idValue->get<int>() == 0) {
		return "You have no active mission.";
	}

	uint32_t missionId = static_cast<uint32_t>(idValue->get<int>());
	const Mission* mission = getMissionById(missionId);
	if (!mission) {
		return "Mission data not found.";
	}

	const auto stageValue = player.kv()->scoped("mission")->scoped("active")->get("stage");
	uint32_t stageIndex = stageValue ? static_cast<uint32_t>(stageValue->get<int>()) : 0;

	if (stageIndex >= mission->stages.size()) {
		return "You have already completed all stages.";
	}

	const MissionStage &stage = mission->stages[stageIndex];

	std::ostringstream oss;
	oss << "\nMission: " << mission->name << ". ";
	oss << "Stage " << stage.id << ": " << stage.name << "\n";

	// Objetivos
	if (!stage.objectives.empty()) {
		oss << "Objectives:\n";
		for (const auto &obj : stage.objectives) {
			switch (obj.type) {
				case MissionObjectiveType::KILL:
					oss << "- Defeat " << obj.amount << "x " << obj.param << "\n";
					break;
				case MissionObjectiveType::COLLECT:
					oss << "- Collect " << obj.amount << "x Item " << obj.param << "\n";
					break;
				case MissionObjectiveType::INVESTIGATE:
					oss << "- Investigate area at " << obj.param << "\n";
					break;
				case MissionObjectiveType::SABOTAGE:
					oss << "- Sabotage or use item " << obj.param << "\n";
					break;
				case MissionObjectiveType::TALK:
					oss << "- Talk to " << obj.param << "\n";
					break;
				case MissionObjectiveType::ESCORT:
					oss << "- Escort " << obj.param << " to " << obj.description << "\n";
					break;
				case MissionObjectiveType::DEFEND:
					oss << "- Defend " << obj.param << "\n";
					break;
				case MissionObjectiveType::CAPTURE:
					oss << "- Capture " << obj.param << "\n";
					break;
				case MissionObjectiveType::SURVIVE:
					oss << "- Survive for " << obj.param << " seconds\n";
					break;
				default:
					oss << "- Unknown objective\n";
					break;
			}
		}
	}

	// Recompensas
	if (!stage.rewards.empty()) {
		oss << "Rewards:\n";
		for (const auto &reward : stage.rewards) {
			switch (reward.type) {
				case MissionRewardType::XP:
					oss << "- " << reward.value << " experience points\n";
					break;
				case MissionRewardType::ITEM:
					oss << "- " << reward.value << "x item " << reward.name << "\n";
					break;
				case MissionRewardType::ATTR_POINTS:
					oss << "- " << reward.value << " attribute points\n";
					break;
				case MissionRewardType::SKILL_TRIES:
					oss << "- " << reward.value << " skill tries for " << reward.name << "\n";
					break;
				case MissionRewardType::PERK:
					oss << "- Unlock perk: " << reward.name << "\n";
					break;
				case MissionRewardType::KVE:
					if (!reward.description.empty()) {
						oss << "- " << reward.description;
					} else {
						oss << "- Set key: " << reward.name << " = " << reward.extraValue;
					}
					break;
				case MissionRewardType::SPELL:
					oss << "- Learn spell: " << reward.name << "\n";
					break;
				default:
					oss << "- Unknown reward\n";
					break;
			}
		}
	}

	return oss.str();
}

bool MissionManager::canStartStage(Player &player, uint32_t missionId, uint32_t stageId) const {
	const Mission* mission = getMissionById(missionId);
	if (!mission || mission->stages.size() <= stageId) {
		return false;
	}

	const MissionStage &stage = mission->stages[stageId];

	for (const auto &[key, value] : stage.kvRequirements) {
		auto kvRef = player.kv();
		std::string scopedKey = key;
		size_t dot;
		while ((dot = scopedKey.find('.')) != std::string::npos) {
			kvRef = kvRef->scoped(scopedKey.substr(0, dot));
			scopedKey = scopedKey.substr(dot + 1);
		}
		const auto current = kvRef->get(scopedKey);
		if (!current.has_value() || current->get<std::string>() != value) {
			return false;
		}
	}

	if (const auto minLevelAttr = stage.requirements.find("level_min"); minLevelAttr != stage.requirements.end()) {
		if (player.getLevel() < static_cast<uint32_t>(std::stoi(minLevelAttr->second))) {
			return false;
		}
	}

	if (const auto skillAttr = stage.requirements.find("skill_name"); skillAttr != stage.requirements.end()) {
		const auto skillLevelAttr = stage.requirements.find("skill_level");
		if (skillLevelAttr != stage.requirements.end()) {
			skills_t skillId = getSkillIdByName(skillAttr->second);
			uint32_t requiredSkillLevel = static_cast<uint32_t>(std::stoi(skillLevelAttr->second));

			uint32_t playerSkillLevel = 0;
			if (skillId == SKILL_NINJUTSU) {
				playerSkillLevel = player.getNinjutsuLevel();
			} else {
				playerSkillLevel = player.getSkillLevel(skillId);
			}

			if (skillId == SKILL_FIRST || playerSkillLevel < requiredSkillLevel) {
				return false;
			}
		}
	}

	if (const auto itemAttr = stage.requirements.find("item_id"); itemAttr != stage.requirements.end()) {
		uint16_t itemId = static_cast<uint16_t>(std::stoi(itemAttr->second));
		const auto countAttr = stage.requirements.find("item_count");
		uint32_t count = countAttr != stage.requirements.end() ? static_cast<uint32_t>(std::stoi(countAttr->second)) : 1;
		if (player.getItemAmount(itemId) < count) {
			return false;
		}
	}

	if (const auto spellAttr = stage.requirements.find("spell_name"); spellAttr != stage.requirements.end()) {
		if (!player.hasLearnedInstantSpell(spellAttr->second)) {
			return false;
		}
	}

	if (const auto vocationAttr = stage.requirements.find("vocation_id"); vocationAttr != stage.requirements.end()) {
		if (player.getVocationId() != static_cast<uint32_t>(std::stoi(vocationAttr->second))) {
			return false;
		}
	}

	if (const auto partyAttr = stage.requirements.find("party_required"); partyAttr != stage.requirements.end()) {
		bool requireParty = partyAttr->second == "true";
		if ((requireParty && !player.getParty()) || (!requireParty && player.getParty())) {
			return false;
		}
	}

	return true;
}

std::string MissionManager::getStageRequirementsStatus(Player &player, uint32_t missionId, uint32_t stageId) const {
	const Mission* mission = getMissionById(missionId);
	if (!mission || mission->stages.size() <= stageId) {
		return "Mission stage not found.";
	}

	const MissionStage &stage = mission->stages[stageId];
	std::ostringstream oss;
	oss << "Stage Requirements:\n";

	for (const auto &[key, value] : stage.kvRequirements) {
		auto kvRef = player.kv();
		std::string scopedKey = key;
		size_t dot;
		while ((dot = scopedKey.find('.')) != std::string::npos) {
			kvRef = kvRef->scoped(scopedKey.substr(0, dot));
			scopedKey = scopedKey.substr(dot + 1);
		}
		const auto current = kvRef->get(scopedKey);
		oss << "- KV " << key << ": " << (current.has_value() && current->get<std::string>() == value ? "OK" : "MISSING") << "\n";
	}

	if (const auto minLevelAttr = stage.requirements.find("level_min"); minLevelAttr != stage.requirements.end()) {
		uint32_t minLevel = static_cast<uint32_t>(std::stoi(minLevelAttr->second));
		oss << "- Level " << minLevel << ": " << (player.getLevel() >= minLevel ? "OK" : "MISSING") << "\n";
	}

	if (const auto skillAttr = stage.requirements.find("skill_name"); skillAttr != stage.requirements.end()) {
		const auto skillLevelAttr = stage.requirements.find("skill_level");
		if (skillLevelAttr != stage.requirements.end()) {
			skills_t skillId = getSkillIdByName(skillAttr->second);
			uint32_t requiredSkillLevel = static_cast<uint32_t>(std::stoi(skillLevelAttr->second));

			uint32_t playerSkillLevel = 0;
			if (skillId == SKILL_NINJUTSU) {
				playerSkillLevel = player.getNinjutsuLevel();
			} else {
				playerSkillLevel = player.getSkillLevel(skillId);
			}

			g_logger().info("[CHECK SKILL] Skill Name: {}, Skill ID: {}, Player Level: {}, Required: {}", skillAttr->second, static_cast<uint32_t>(skillId), playerSkillLevel, requiredSkillLevel);

			oss << "- Skill " << skillAttr->second << " " << requiredSkillLevel << ": "
				<< (skillId != SKILL_FIRST && playerSkillLevel >= requiredSkillLevel ? "OK" : "MISSING") << "\n";
		}
	}

	if (const auto itemAttr = stage.requirements.find("item_id"); itemAttr != stage.requirements.end()) {
		uint16_t itemId = static_cast<uint16_t>(std::stoi(itemAttr->second));
		const auto countAttr = stage.requirements.find("item_count");
		uint32_t count = countAttr != stage.requirements.end() ? static_cast<uint32_t>(std::stoi(countAttr->second)) : 1;
		const ItemType &it = Item::items.getItemType(itemId);
		std::string itemName = it.name.empty() ? std::to_string(itemId) : it.name;
		oss << "- Item " << itemName << " x" << count << ": " << (player.getItemAmount(itemId) >= count ? "OK" : "MISSING") << "\n";
	}

	if (const auto spellAttr = stage.requirements.find("spell_name"); spellAttr != stage.requirements.end()) {
		oss << "- Spell " << spellAttr->second << ": " << (player.hasLearnedInstantSpell(spellAttr->second) ? "OK" : "MISSING") << "\n";
	}

	if (const auto vocationAttr = stage.requirements.find("vocation_id"); vocationAttr != stage.requirements.end()) {
		oss << "- Vocation ID " << vocationAttr->second << ": " << (player.getVocationId() == static_cast<uint32_t>(std::stoi(vocationAttr->second)) ? "OK" : "MISSING") << "\n";
	}

	if (const auto partyAttr = stage.requirements.find("party_required"); partyAttr != stage.requirements.end()) {
		bool requiresParty = partyAttr->second == "true";
		oss << "- Party: " << ((requiresParty && player.getParty()) || (!requiresParty && !player.getParty()) ? "OK" : "MISSING") << "\n";
	}

	return oss.str();
}

std::string MissionManager::formatRequirementsForStage(Player &player, uint32_t missionId, uint32_t stageId) const {
	const Mission* mission = getMissionById(missionId);
	if (!mission || mission->stages.size() <= stageId) {
		return "Invalid mission or stage.";
	}

	const MissionStage &stage = mission->stages[stageId];
	std::ostringstream oss;
	bool anyRequirement = false;

	for (const auto &[key, value] : stage.requirements) {
		if (key == "level") {
			uint32_t requiredLevel = static_cast<uint32_t>(std::stoi(value));
			if (player.getLevel() < requiredLevel) {
				oss << "- Reach level " << requiredLevel << "\n";
				anyRequirement = true;
			}
		} else if (key == "vocation_id") {
			uint32_t vocationId = static_cast<uint32_t>(std::stoi(value));
			if (!player.getVocationId() || player.getVocationId() != vocationId) {
				oss << "- Be from vocation ID " << vocationId << "\n";
				anyRequirement = true;
			}
		} else if (key == "skill_name") {
			const auto skillLevelIt = stage.requirements.find("skill_level");
			if (skillLevelIt != stage.requirements.end()) {
				skills_t skillId = getSkillIdByName(value);
				uint32_t requiredSkillLevel = static_cast<uint32_t>(std::stoi(skillLevelIt->second));

				uint32_t playerSkillLevel = 0;
				if (skillId == SKILL_NINJUTSU) {
					playerSkillLevel = player.getNinjutsuLevel();
				} else {
					playerSkillLevel = player.getSkillLevel(skillId);
				}

				if (playerSkillLevel < requiredSkillLevel) {
					oss << "- Have skill '" << value << "' at level " << requiredSkillLevel << "\n";
					anyRequirement = true;
				}
			}
		} else if (key == "item_id") {
			const auto countIt = stage.requirements.find("item_count");
			uint16_t itemId = 0;
			if (std::all_of(value.begin(), value.end(), ::isdigit)) {
				itemId = static_cast<uint16_t>(std::stoi(value));
			} else {
				itemId = Item::items.getItemIdByName(value);
			}

			if (itemId > 0) {
				uint32_t requiredCount = countIt != stage.requirements.end() ? static_cast<uint32_t>(std::stoi(countIt->second)) : 1;
				if (player.getItemAmount(itemId) < requiredCount) {
					const ItemType &it = Item::items.getItemType(itemId);
					std::string itemName = it.name.empty() ? std::to_string(itemId) : it.name;
					oss << "- Bring " << requiredCount << "x " << itemName << "\n";
					anyRequirement = true;
				}
			}
		} else if (key == "spell_name") {
			if (!player.hasLearnedInstantSpell(value)) {
				oss << "- Learn the spell '" << value << "'\n";
				anyRequirement = true;
			}
		} else if (key == "party_required") {
			bool requiresParty = (value == "true");
			if (requiresParty && !player.getParty()) {
				oss << "- Be in a party";
				anyRequirement = true;
			}
		}
	}

	return oss.str();
}

std::string MissionManager::getStageProgress(Player &player) const {

	auto missionScope = player.kv()->scoped("mission"); // <- primeiro

	uint32_t missionId = getActiveMissionId(player);
	const Mission* mission = getMissionById(missionId);
	if (!mission) {
		return "You have no active mission.";
	}

	if (!mission->repeatable && missionScope->scoped("active")->get("completed").value_or(false)) {
		return "You have completed this mission. Your help is no longer needed.";
	}

	uint32_t stageId = getActiveMissionStageIndex(player);
	if (mission->stages.size() <= stageId) {
		return "You have no active mission stage.";
	}

	const MissionStage &stage = mission->stages[stageId];
	std::ostringstream oss;
	oss << "Stage Progress:\n";

	for (const auto &obj : stage.objectives) {
		auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");
		std::string key = asLowerCaseString(obj.param);

		switch (obj.type) {
			case MissionObjectiveType::KILL: {
				int currentKills = objectivesKV->get(key).value_or(0);
				oss << "- Kill " << key << ": " << currentKills << "/" << obj.amount << "\n";
				break;
			}
			case MissionObjectiveType::COLLECT: {
				uint16_t itemId = static_cast<uint16_t>(std::stoi(obj.param));
				uint32_t collected = player.getItemAmount(itemId);
				oss << "- Collect item " << itemId << ": " << collected << "/" << obj.amount << "\n";
				break;
			}
			case MissionObjectiveType::TALK: {
				auto talkedKV = player.kv()->scoped("mission")->scoped("talked");
				std::string talkKey = asLowerCaseString(obj.param);
				bool talked = talkedKV->get(talkKey).value_or(false);
				oss << "- Talk to " << obj.param << ": " << (talked ? "DONE" : "MISSING") << "\n";
				break;
			}
			case MissionObjectiveType::INVESTIGATE:
			case MissionObjectiveType::SABOTAGE: {
				bool investigated = objectivesKV->get(key).value_or(false);
				oss << "- Investigate/use " << key << ": " << (investigated ? "DONE" : "MISSING") << "\n";
				break;
			}
			case MissionObjectiveType::ESCORT: {
				bool escorted = objectivesKV->get(key).value_or(false);
				oss << "- Escort " << key << ": " << (escorted ? "DONE" : "MISSING") << "\n";
				break;
			}
			case MissionObjectiveType::DEFEND: {
				bool defended = objectivesKV->get(key).value_or(false);
				oss << "- Defend " << key << ": " << (defended ? "DONE" : "MISSING") << "\n";
				break;
			}
			case MissionObjectiveType::CAPTURE: {
				bool captured = objectivesKV->get(key).value_or(false);
				oss << "- Capture " << key << ": " << (captured ? "DONE" : "MISSING") << "\n";
				break;
			}
			case MissionObjectiveType::SURVIVE: {
				bool survived = objectivesKV->get(key).value_or(false);
				oss << "- Survive for " << obj.param << " seconds: " << (survived ? "DONE" : "MISSING") << "\n";
				break;
			}
			case MissionObjectiveType::USE_ON: {
				auto useonKV = player.kv()->scoped("mission")->scoped("useon");
				int currentUses = useonKV->get(key).value_or(0);
				oss << "- Use item on " << key << ": " << currentUses << "/" << obj.amount << "\n";
				break;
			}
			default:
				oss << "- Unknown objective type.\n";
				break;
		}
	}
	return oss.str();
}

const MissionStage* MissionManager::getActiveMissionStage(Player &player) const {
	uint32_t missionId = getActiveMissionId(player);
	if (missionId == 0) {
		return nullptr;
	}

	const Mission* mission = getMissionById(missionId);
	if (!mission) {
		return nullptr;
	}

	uint32_t stageId = getActiveMissionStageIndex(player);
	if (stageId >= mission->stages.size()) {
		return nullptr;
	}

	return &mission->stages[stageId];
}

void MissionManager::onUseItem(Player &player, uint16_t itemId) const {
	const MissionStage* stage = getActiveMissionStage(player);
	if (!stage) {
		return;
	}

	std::string itemKey = std::to_string(itemId);
	auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");

	bool updated = false;

	for (const auto &objective : stage->objectives) {
		if ((objective.type == MissionObjectiveType::SABOTAGE || objective.type == MissionObjectiveType::INVESTIGATE) && objective.param == itemKey) {

			if (objectivesKV->get(itemKey).has_value()) {
				// Já completou esse objetivo
				return;
			}

			objectivesKV->set(itemKey, true);
			updated = true;

			player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("Updated objective: Used item {}!", itemKey));

			break;
		}
	}

	/*   if (updated) {
	      checkStageCompletion(player);
	  } */
}

void MissionManager::onKill(Player &player, const std::string &monsterName) const {
	const MissionStage* stage = getActiveMissionStage(player);
	if (!stage) {
		return;
	}

	auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");
	std::string monsterKey = asLowerCaseString(monsterName);

	bool updated = false;

	for (const auto &objective : stage->objectives) {
		if (objective.type == MissionObjectiveType::KILL && asLowerCaseString(objective.param) == monsterKey) {

			int current = objectivesKV->get(monsterKey).value_or(0);

			if (current >= static_cast<int>(objective.amount)) {
				// Já completou esse objetivo
				return;
			}

			int newValue = current + 1;
			objectivesKV->set(monsterKey, newValue);

			updated = true;

			player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("Updated objective: {} ({}/{})", objective.param, newValue, objective.amount));

			break; // Encontrou e atualizou, não precisa continuar o loop
		}
	}

	/*   if (updated) {
	      checkStageCompletion(player);
	  } */
}

void MissionManager::checkStageCompletion(Player &player) const {
	const Mission* mission = getMissionById(getActiveMissionId(player));
	if (!mission) {
		return;
	}

	uint32_t currentStageId = getActiveMissionStageIndex(player);
	if (currentStageId >= mission->stages.size()) {
		return;
	}

	const MissionStage &stage = mission->stages[currentStageId];
	if (stage.objectives.empty()) {
		return;
	}

	// Usa a função unificada para validar e já remover itens se necessário
	if (!hasCompletedAllObjectives(player, stage, true)) {
		return;
	}

	uint32_t nextStage = currentStageId + 1;
	if (nextStage < mission->stages.size()) {
		setActiveMissionStage(player, nextStage);
		player.kv()->scoped("mission")->scoped("objectives")->remove("");
	} else {
		if (!mission->repeatable) {
			player.kv()->scoped("mission")->scoped("active")->set("completed", true);
		}
		player.kv()->scoped("mission")->scoped("objectives")->remove("");
	}
	applyRewards(player, stage);
}

void MissionManager::onCollectItem(Player &player, uint16_t itemId, uint32_t amount) const {
	const MissionStage* stage = getActiveMissionStage(player);
	if (!stage) {
		return;
	}

	auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");

	bool updated = false;
	std::string itemKey = std::to_string(itemId);

	for (const auto &objective : stage->objectives) {
		if (objective.type == MissionObjectiveType::COLLECT && objective.param == itemKey) {
			int current = objectivesKV->get(itemKey).value_or(0);

			if (current >= static_cast<int>(objective.amount)) {
				return; // Já completou
			}

			int newValue = current + static_cast<int>(amount);
			if (newValue > static_cast<int>(objective.amount)) {
				newValue = static_cast<int>(objective.amount); // Não passar do objetivo máximo
			}

			objectivesKV->set(itemKey, newValue);
			updated = true;

			player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("Updated collect objective: {} ({}/{})", itemKey, newValue, objective.amount));

			break;
		}
	}

	/* if (updated) {
	    checkStageCompletion(player);
	} */
}

void MissionManager::onUseOnTarget(Player &player, uint16_t itemId, const std::string &targetName) const {
	const MissionStage* stage = getActiveMissionStage(player);
	if (!stage) {
		return;
	}

	auto objectivesKV = player.kv()->scoped("mission")->scoped("useon");
	std::string targetKey = asLowerCaseString(targetName);

	bool updated = false;

	for (const auto &objective : stage->objectives) {
		if (objective.type == MissionObjectiveType::USE_ON && asLowerCaseString(objective.param) == targetKey && objective.itemId == itemId) {

			int current = objectivesKV->get(targetKey).value_or(0);

			if (current >= static_cast<int>(objective.amount)) {
				// Já completou esse objetivo
				return;
			}

			int newValue = current + 1;
			objectivesKV->set(targetKey, newValue);

			updated = true;

			player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("Updated USE_ON objective: {} ({}/{})", objective.param, newValue, objective.amount));

			break;
		}
	}

	/*    if (updated) {
	       checkStageCompletion(player);
	   } */
}

void MissionManager::completeTalkObjective(Player &player, const std::string &npcName) const {
	const MissionStage* stage = getActiveMissionStage(player);
	if (!stage) {
		return;
	}

	auto objectivesKV = player.kv()->scoped("mission")->scoped("talked");
	std::string key = asLowerCaseString(npcName);

	for (const auto &objective : stage->objectives) {
		if (objective.type == MissionObjectiveType::TALK && asLowerCaseString(objective.param) == key) {

			if (objectivesKV->get(key).value_or(false)) {
				// Já falou com este NPC, não precisa marcar de novo
				return;
			}

			objectivesKV->set(key, true);

			player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("You talked to {} for your mission.", npcName));

			/*       checkStageCompletion(player); */
			break;
		}
	}
}

bool MissionManager::hasCompletedAllObjectives(Player &player, const MissionStage &stage, bool removeCollectItems) const {
	if (stage.objectives.empty()) {
		return true;
	}

	auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");
	auto talkedKV = player.kv()->scoped("mission")->scoped("talked");
	auto useonKV = player.kv()->scoped("mission")->scoped("useon");

	for (const auto &objective : stage.objectives) {
		std::string key = asLowerCaseString(objective.param);

		switch (objective.type) {
			case MissionObjectiveType::KILL: {
				int progress = objectivesKV->get(key).value_or(0);
				if (progress < static_cast<int>(objective.amount)) {
					return false;
				}
				break;
			}

			case MissionObjectiveType::USE_ON: {
				int progress = useonKV->get(key).value_or(0);
				if (progress < static_cast<int>(objective.amount)) {
					return false;
				}
				break;
			}

			case MissionObjectiveType::COLLECT: {
				uint16_t itemId = static_cast<uint16_t>(std::stoi(objective.param));
				uint32_t required = objective.amount;

				if (!player.removeItemCountById(itemId, required, false)) {
					return false;
				}

				if (removeCollectItems) {
					player.removeItemCountById(itemId, required, true);
					objectivesKV->set(key, static_cast<int>(required));
					player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("You delivered {}x of item {} for the mission.", required, itemId));
				}
				break;
			}

			case MissionObjectiveType::INVESTIGATE:
			case MissionObjectiveType::SABOTAGE: {
				if (!objectivesKV->get(key).value_or(false)) {
					return false;
				}
				break;
			}

			case MissionObjectiveType::TALK:
			case MissionObjectiveType::ESCORT:
			case MissionObjectiveType::DEFEND:
			case MissionObjectiveType::CAPTURE:
			case MissionObjectiveType::SURVIVE: {
				if (!talkedKV->get(key).value_or(false)) {
					return false;
				}
				break;
			}
		}
	}

	return true;
}

void MissionManager::applyRewards(Player &player, const MissionStage &stage) const {
	for (const auto &reward : stage.rewards) {
		switch (reward.type) {
			case MissionRewardType::XP:
				player.addExperience(nullptr, reward.value, true);
				break;

			case MissionRewardType::ITEM: {
				uint16_t itemId = 0;
				if (std::all_of(reward.name.begin(), reward.name.end(), ::isdigit)) {
					itemId = static_cast<uint16_t>(std::stoi(reward.name));
				} else {
					itemId = Item::items.getItemIdByName(reward.name);
				}

				if (itemId == 0) {
					g_logger().warn("[MissionManager] Invalid item '{}' in reward.", reward.name);
					break;
				}

				const ItemType &it = Item::items.getItemType(itemId);
				uint16_t subType = reward.value;
				if (!it.stackable) {
					subType = 1;
				}

				const auto &playerInbox = player.getInbox();
				const auto &item = Item::CreateItem(itemId, subType);

				if (g_game().internalAddItem(playerInbox, item, INDEX_WHEREEVER, FLAG_NOLIMIT) != RETURNVALUE_NOERROR) {
					g_logger().warn("[MissionManager] Failed to add item {} to inbox of player {}", itemId, player.getName());
				}
				break;
			}

			case MissionRewardType::ATTR_POINTS:
				/* player.attributes().addStatusPoints(reward.value); */
				break;

			case MissionRewardType::SKILL_TRIES: {
				/*   skills_t skillId = getSkillIdByName(reward.name);
				  if (skillId != SKILL_FIRST) {
				      player.addSkillTries(skillId, reward.value);
				  } */
				break;
			}

			case MissionRewardType::PERK:
				player.kv()->scoped("perks")->set(reward.name, true);
				break;

			case MissionRewardType::KVE:
				player.kv()->set(reward.name, reward.extraValue);
				break;

			case MissionRewardType::SPELL: {
				if (!reward.name.empty()) {
					player.learnInstantSpell(reward.name);
					player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("You have learned a new spell: {}!", reward.name));
				}
				break;
			}

			default:
				g_logger().warn("[MissionManager] Unknown reward type.");
				break;
		}
	}
}
