#include "creatures/players/pet/pet_data.hpp"
#include "database/database.hpp"
#include "fmt/format.h"

#include "creatures/monsters/monster.hpp"
#include "creatures/monsters/monsters.hpp"
#include "game/game.hpp"
#include "map/spectators.hpp"
#include "game/scheduling/dispatcher.hpp"

std::unordered_map<Player*, std::unordered_map<uint16_t, PetData>> PetManager::pets;

void PetManager::setActivePet(Player* player, uint16_t petId, bool summoned) {
    if (!player) return;
    player->kv()->set("active_pet_id", petId);
    player->kv()->set("active_pet_summoned", summoned);
}

uint16_t PetManager::getActivePetId(Player* player) {
    if (!player) return 0;
    int rawPetId = player->kv()->get("active_pet_id").value_or(0);
    return static_cast<uint16_t>(rawPetId);
}

bool PetManager::isPetSummoned(Player* player) {
    if (!player) return false;
    return player->kv()->get("active_pet_summoned").value_or(false);
}

PetManager &PetManager::getInstance() {
    static PetManager instance;
    return instance;
}

void PetData::reset() {
    level = 1;
    experience = 0;
    evolutionStage = 1;
    health = 100;
    maxHealth = 100;
    chakra = 50;
    maxChakra = 50;
    attack = 15;
    defense = 10;
    speed = 100;
}

void PetData::addExperience(uint64_t amount, uint32_t playerLevel) {
    experience += amount;
	while (checkLevelUp(playerLevel)) {
        level++;
        health += 10;
        maxHealth += 10;
        chakra += 5;
        maxChakra += 5;
        attack += 2;
        defense += 2;
        speed += 1;
    }
}

bool PetData::checkLevelUp(uint32_t playerLevel) {
	const uint64_t required = getRequiredExperience(level + 1);
	if (experience >= required) {
		level++;
		applyBaseStatsFromLevel(playerLevel); // ✅ aplica os status balanceados
		return true;
	}
	return false;
}

uint64_t PetData::getRequiredExperience(uint32_t lvl) const {
    return lvl * lvl * 50;
}

void PetData::evolve() {
    if (evolutionStage < 3) {
        evolutionStage++;
        applyEvolutionBonus();
    }
}

void PetData::applyEvolutionBonus() {
    switch (evolutionStage) {
        case 2:
            maxHealth += 100;
            maxChakra += 50;
            attack += 10;
            defense += 10;
            speed += 20;
            break;
        case 3:
            maxHealth += 150;
            maxChakra += 75;
            attack += 15;
            defense += 15;
            speed += 30;
            break;
        default:
            break;
    }
}

PetData &PetManager::getPet(Player* player, uint16_t petId) {
    return pets[player][petId];
}

bool PetManager::hasPet(Player* player, uint16_t petId) {
    auto it = pets.find(player);
    return it != pets.end() && it->second.find(petId) != it->second.end();
}

void PetManager::removePet(Player* player, uint16_t petId) {
    auto it = pets.find(player);
    if (it != pets.end()) {
        it->second.erase(petId);
    }
}

void PetData::loadFromDatabase(uint32_t playerId, uint16_t petId) {
    Database &db = Database::getInstance();
    std::ostringstream query;
    query << "SELECT * FROM `player_pet_data` WHERE `player_id` = " << playerId << " AND `pet_id` = " << petId;

    DBResult_ptr result = db.storeQuery(query.str());
    if (!result) {
        return;
    }

    name = result->getString("name");
    level = result->getNumber<uint32_t>("level");
    experience = result->getNumber<uint64_t>("experience");
    evolutionStage = result->getNumber<uint32_t>("evolution");

    health = result->getNumber<int32_t>("health");
    maxHealth = result->getNumber<int32_t>("max_health");
    chakra = result->getNumber<int32_t>("chakra");
    maxChakra = result->getNumber<int32_t>("max_chakra");
    attack = result->getNumber<int32_t>("attack");
    defense = result->getNumber<int32_t>("defense");
    speed = result->getNumber<int32_t>("speed");
}

void PetData::saveToDatabase(uint32_t playerId, uint16_t petId) const {
    Database &db = Database::getInstance();
    std::ostringstream query;

    query << "INSERT INTO `player_pet_data` "
          << "(`player_id`, `pet_id`, `level`, `experience`, `evolution`, `health`, `max_health`, `chakra`, `max_chakra`, `attack`, `defense`, `speed`, `name`) VALUES ("
          << playerId << ", "
          << petId << ", "
          << level << ", "
          << experience << ", "
          << evolutionStage << ", "
          << health << ", "
          << maxHealth << ", "
          << chakra << ", "
          << maxChakra << ", "
          << attack << ", "
          << defense << ", "
          << speed << ", "
          << db.escapeString(name) << ") "
          << "ON DUPLICATE KEY UPDATE "
          << "`level` = VALUES(`level`), "
          << "`experience` = VALUES(`experience`), "
          << "`evolution` = VALUES(`evolution`), "
          << "`health` = VALUES(`health`), "
          << "`max_health` = VALUES(`max_health`), "
          << "`chakra` = VALUES(`chakra`), "
          << "`max_chakra` = VALUES(`max_chakra`), "
          << "`attack` = VALUES(`attack`), "
          << "`defense` = VALUES(`defense`), "
          << "`speed` = VALUES(`speed`), "
          << "`name` = VALUES(`name`)";

    db.executeQuery(query.str());
}

void PetManager::loadPet(Player* player, uint16_t petId) {
    if (!player) return;
    auto& pet = getPet(player, petId);
    pet.loadFromDatabase(player->getGUID(), petId);
}

void PetManager::savePet(Player* player, uint16_t petId) {
    if (!player) return;
    auto it = pets.find(player);
    if (it != pets.end() && it->second.find(petId) != it->second.end()) {
        it->second[petId].saveToDatabase(player->getGUID(), petId);
    }
}

void PetManager::togglePet(Player* player, uint16_t petId) {
	if (!player) return;

	// 🔄 Cooldown de 600ms entre chamadas
	static std::unordered_map<Player*, uint64_t> lastToggleTime;
	uint64_t now = OTSYS_TIME();
	uint64_t last = lastToggleTime[player];
	if (now - last < 600) {
		player->sendTextMessage(MESSAGE_LOOK, "Espere um pouco antes de alternar o pet novamente.");
		return;
	}
	lastToggleTime[player] = now;

	if (!hasPet(player, petId)) {
		player->sendTextMessage(MESSAGE_EVENT_ADVANCE, "Você ainda não possui este pet.");
		return;
	}

	uint16_t currentId = getActivePetId(player);

	// 🟥 Se o mesmo pet está invocado → remove ele
	if (currentId == petId && isPetSummoned(player)) {
		const auto spectators = Spectators().find<Creature>(player->getPosition(), true);
		for (const auto& creature : spectators) {
			const auto& monster = creature->getMonster();
			if (monster && creature->getMaster().get() == player) {
				g_game().addMagicEffect(creature->getPosition(), CONST_ME_POFF);
				g_game().removeCreature(creature);
				break;
			}
		}

		setActivePet(player, 0, false);
		getPet(player, petId).saveToDatabase(player->getGUID(), petId);
		player->sendTextMessage(MESSAGE_LOOK, fmt::format("{} foi embora!", getPet(player, petId).getNamePet()));
		return;
	}

	// 🟨 Se outro pet está invocado → remove ele antes
	if (isPetSummoned(player)) {
		const auto spectators = Spectators().find<Creature>(player->getPosition(), true);
		for (const auto& creature : spectators) {
			const auto& monster = creature->getMonster();
			if (monster && creature->getMaster().get() == player) {
				g_game().addMagicEffect(creature->getPosition(), CONST_ME_POFF);
				g_game().removeCreature(creature);
				break;
			}
		}
		setActivePet(player, 0, false);
	}

	// 🟩 Invoca novo pet
	setActivePet(player, petId, true);
	player->sendTextMessage(MESSAGE_LOOK, fmt::format("{} apareceu!", getPet(player, petId).getNamePet()));

	Position pos = player->getPosition();
	pos.x += 1;

	std::shared_ptr<Monster> pet = Monster::createMonster("akamaru");
	if (pet && g_game().placeCreature(pet, pos, false, true)) {
		const auto& master = g_game().getCreatureByID(player->getID());
		if (master) {
			pet->setMaster(master);
		}
		updatePet(player, pet, petId);
	} else {
		player->sendTextMessage(MESSAGE_LOOK, "Não foi possível invocar o pet.");
	}
}


bool PetManager::isPetHidden(Player* player) const {
    return player->kv()->get("pet_hidden").value_or(false);
}

void PetManager::updatePet(Player* player, const std::shared_ptr<Monster>& pet, uint16_t petId) {
	if (!player || !pet) return;

	auto& data = getPet(player, petId);

	pet->setName(fmt::format("{} [Lv. {}]", data.getNamePet(), data.getLevel()));

	// ⚠️ Corrige os valores de maxHealth/maxChakra SEM ACUMULAR
	int32_t currentMaxHp = pet->getMaxHealth();
	if (currentMaxHp != data.getMaxHealth()) {
		pet->changeMaxHealth(data.getMaxHealth() - currentMaxHp);
	}

	int32_t currentMaxMana = pet->getMaxMana();
	if (currentMaxMana != data.getMaxChakra()) {
		pet->changeMaxMana(data.getMaxChakra() - currentMaxMana);
	}

	// ✅ Sincroniza vida e chakra com o banco
	pet->changeHealth(data.getHealth() - pet->getHealth());
	pet->changeMana(data.getChakra() - pet->getMana());

	// ✅ Velocidade
	pet->setBaseSpeed(data.getSpeed());
	pet->setSpeed(data.getSpeed());

	// ✅ Salva o estado real no banco de dados
	data.setHealth(pet->getHealth());
	data.setChakra(pet->getMana());

    data.applyBaseStatsFromLevel(player->getLevel());
}


std::shared_ptr<Monster> PetManager::getPetByType(Player* player, const std::string& type) {
    if (!player) return nullptr;

    const auto specs = Spectators().find<Creature>(player->getPosition(), false);
    for (const auto& creature : specs) {
        if (auto monster = creature->getMonster()) {
            if (creature->getMaster().get() == player && toLowerCase(monster->getName()) == toLowerCase(type)) {
                return std::static_pointer_cast<Monster>(creature);
            }
        }
    }

    return nullptr;
}

std::shared_ptr<Monster> PetManager::getAkamaru(Player* player) {
    return getPetByType(player, "akamaru");
}

std::shared_ptr<Monster> PetManager::getPetInstance(Player* player, uint16_t petId) {
    const auto spectators = Spectators().find<Creature>(player->getPosition(), true);
    for (const auto& creature : spectators) {
        const auto& monster = creature->getMonster();
        if (monster && creature->getMaster().get() == player) {
            const std::string baseName = getPet(player, petId).getNamePet();
            if (monster->getName().starts_with(baseName)) {
                return std::static_pointer_cast<Monster>(monster);
            }
        }
    }
    return nullptr;
}

bool PetManager::setPetName(Player* player, uint16_t petId, const std::string& newName) {
    if (!player || newName.empty()) {
        return false;
    }

    if (newName.length() < 3 || newName.length() > 12) {
        player->sendTextMessage(MESSAGE_EVENT_ADVANCE, "Nome inválido. Deve ter entre 3 e 12 caracteres.");
        return false;
    }

    PetData& pet = getPet(player, petId);
    pet.setNamePet(newName);

    // Atualiza em tempo real se estiver invocado
    const auto specs = Spectators().find<Creature>(player->getPosition(), true);
    for (const auto& creature : specs) {
        const auto& monster = creature->getMonster();
        if (creature->getMaster().get() == player && monster) {
            uint16_t currentPetId = getActivePetId(player);
            if (currentPetId == petId) {
                monster->setName(fmt::format("{} [Lv. {}]", newName, pet.getLevel()));
            }
        }
    }

    return true;
}

uint16_t PetManager::generateNewPetId(Player* player) {
	if (!player) return 1;

	auto it = pets.find(player);
	if (it == pets.end()) return 1;

	uint16_t id = 1;
	while (it->second.contains(id)) {
		id++;
	}

	return id;
}


void PetManager::loadAllPets(Player* player) {
    if (!player) return;

    Database& db = Database::getInstance();
    auto result = db.storeQuery(fmt::format("SELECT pet_id FROM player_pet_data WHERE player_id = {}", player->getGUID()));
    if (!result) return;

    do {
        uint16_t petId = result->getNumber<uint16_t>("pet_id");
        auto& pet = getPet(player, petId);
        pet.loadFromDatabase(player->getGUID(), petId);
    } while (result->next());
} 


void PetManager::saveAllPets(Player* player) {
    if (!player) return;

    auto it = pets.find(player);
    if (it == pets.end()) return;

    for (const auto& [petId, petData] : it->second) {
        petData.saveToDatabase(player->getGUID(), petId);
    }
}

std::string PetManager::getPetInfo(Player* player, uint16_t petId) {
    if (!player || !hasPet(player, petId)) {
        return "Pet não encontrado.";
    }

    const auto& pet = getPet(player, petId);
    return fmt::format(
        "Pet ID: {}\n"
        "Nome: {}\n"
        "Vida: {}/{}\n"
        "Chakra: {}/{}\n"
        "Ataque: {}\n"
        "Defesa: {}\n"
        "Velocidade: {}\n"
        "Level: {} ({}/{} XP)\n"
        "Evolução: {}",
        petId,
        pet.getNamePet(),
        pet.getHealth(), pet.getMaxHealth(),
        pet.getChakra(), pet.getMaxChakra(),
        pet.getAttack(),
        pet.getDefense(),
        pet.getSpeed(),
        pet.getLevel(), pet.getExperience(), pet.getRequiredExperience(pet.getLevel() + 1),
        pet.getEvolutionStage()
    );
}

void PetData::applyBaseStatsFromLevel(uint32_t playerLevel) {
	// Fórmulas balanceadas
	const float healthScale = 10.0f;
	const float chakraScale = 7.5f;
	const float atkScale = 2.5f;
	const float defScale = 2.0f;
	const float speedScale = 0.5f;

	maxHealth = 100 + static_cast<int32_t>((level + playerLevel) * healthScale);
	maxChakra = 50 + static_cast<int32_t>((level + playerLevel) * chakraScale);
	attack = 15 + static_cast<int32_t>((level + playerLevel) * atkScale);
	defense = 10 + static_cast<int32_t>((level + playerLevel) * defScale);
	speed = 100 + static_cast<int32_t>((level + playerLevel) * speedScale);

	// Garante que HP/Chakra atual nunca excedam o novo máximo
	health = std::min(health, maxHealth);
	chakra = std::min(chakra, maxChakra);
}
void PetManager::movePetTo(Player* player, uint16_t petId, const Position& targetPos) {
	if (!player || !isPetSummoned(player))
		return;

	const auto& pet = getPetInstance(player, petId);
	if (!pet || pet->isRemoved())
		return;

	if (pet->getPosition() == targetPos) {
		player->sendTextMessage(MESSAGE_EVENT_ADVANCE, "O pet já está nessa posição.");
		return;
	}

	std::vector<Direction> path;
	path.reserve(32);

	bool found = pet->getPathTo(targetPos, path, 0, 0, true, true, 50);
	if (!found || path.empty()) {
		player->sendTextMessage(MESSAGE_EVENT_ADVANCE, "Akamaru não conseguiu andar.");
		return;
	}

	pet->setFollowCreature(nullptr);
	player->kv()->set("pet_manual_move", true);

	const uint32_t delay = std::max<uint32_t>(pet->getStepSpeed(), 100); // Garante suavidade mínima
	auto petWeak = std::weak_ptr<Creature>(pet);

	auto walkStep = std::make_shared<std::function<void(size_t)>>();

	*walkStep = [walkStep, path, petWeak, player, delay](size_t index) {
		const auto& petLocked = petWeak.lock();

		if (!petLocked || petLocked->isRemoved() || !player || player->isRemoved())
			return;

		if (index >= path.size()) {
			player->kv()->set("pet_manual_move", false);
			player->sendTextMessage(MESSAGE_EVENT_ADVANCE, "Akamaru chegou ao destino.");
			return;
		}

		Direction dir = path[index];
		g_game().internalCreatureTurn(petLocked, dir);

		if (g_game().internalMoveCreature(petLocked, dir, FLAG_IGNOREBLOCKITEM | FLAG_IGNOREBLOCKCREATURE | FLAG_NOLIMIT) != RETURNVALUE_NOERROR) {
			player->kv()->set("pet_manual_move", false);
			player->sendTextMessage(MESSAGE_EVENT_ADVANCE, "Akamaru não conseguiu continuar o caminho.");
			return;
		}

		g_dispatcher().scheduleEvent(delay, [walkStep, index]() {
			(*walkStep)(index + 1);
		}, "Pet::stepWalk");
	};

	(*walkStep)(0);
}
