#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include "creatures/players/player.hpp"

class PetData final {
public:
    PetData() = default;

    void reset();

    const std::string &getNamePet() const { return name; }
    void setNamePet(const std::string &newName) { name = newName; }

    // Nível e XP
    void addExperience(uint64_t amount, uint32_t playerLevel);
    bool checkLevelUp(uint32_t playerLevel);

    uint32_t getLevel() const { return level; }
    uint64_t getExperience() const { return experience; }

    // Evolução
    void evolve();
    uint32_t getEvolutionStage() const { return evolutionStage; }

    // Status
    int32_t getHealth() const { return health; }
    int32_t getMaxHealth() const { return maxHealth; }
    int32_t getMaxChakra() const { return maxChakra; }
    int32_t getChakra() const { return chakra; }
    int32_t getAttack() const { return attack; }
    int32_t getDefense() const { return defense; }
    int32_t getSpeed() const { return speed; }

    void setHealth(int32_t value) { health = value; }
    void setChakra(int32_t value) { chakra = value; }
    void setMaxHealth(int32_t value) { maxHealth = value; }
    void setMaxChakra(int32_t value) { maxChakra = value; }

    uint64_t getRequiredExperience(uint32_t level) const;

    void loadFromDatabase(uint32_t playerId, uint16_t petId);
    void saveToDatabase(uint32_t playerId, uint16_t petId) const;

    void applyEvolutionBonus();
    void applyBaseStatsFromLevel(uint32_t playerLevel);

private:
    std::string name = "Akamaru";
    uint32_t level = 1;
    uint64_t experience = 0;

    uint32_t evolutionStage = 1;

    int32_t health = 100;
    int32_t maxHealth = 100;
    int32_t chakra = 50;
    int32_t maxChakra = 50;
    int32_t attack = 15;
    int32_t defense = 10;
    int32_t speed = 100;
};

class PetManager final {
public:
    static PetManager &getInstance();

    static PetData &getPet(Player* player, uint16_t petId);
    static bool hasPet(Player* player, uint16_t petId);
    static void removePet(Player* player, uint16_t petId);

    void togglePet(Player* player, uint16_t petId);
    static void updatePet(Player* player, const std::shared_ptr<Monster>& pet, uint16_t petId);

    uint16_t generateNewPetId(Player* player);
    void loadAllPets(Player* player);
    void saveAllPets(Player* player);

    bool isPetHidden(Player* player) const;
    bool setPetName(Player* player, uint16_t petId, const std::string& newName);

    static std::shared_ptr<Monster> getAkamaru(Player* player);
    static std::shared_ptr<Monster> getPetByType(Player* player, const std::string& type);
    static std::shared_ptr<Monster> getPetInstance(Player* player, uint16_t petId);

    void loadPet(Player* player, uint16_t petId);
    void savePet(Player* player, uint16_t petId);

    static void setActivePet(Player* player, uint16_t petId, bool summoned);
    static uint16_t getActivePetId(Player* player);
    static bool isPetSummoned(Player* player);

    static std::string getPetInfo(Player* player, uint16_t petId);
    void movePetTo(Player* player, uint16_t petId, const Position& targetPos);

private:
    static std::unordered_map<Player*, std::unordered_map<uint16_t, PetData>> pets;


};

constexpr auto g_pet = PetManager::getInstance;
