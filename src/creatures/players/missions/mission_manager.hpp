#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstdint>

#include "creatures/monsters/monsters.hpp"

class Player;

enum MissionObjectiveType : uint8_t {
	KILL,        // Matar monstros
	COLLECT,     // Coletar itens
	INVESTIGATE,          // Ir até determinado local / INVESTIGAR
	SABOTAGE,         // Usar item ou objeto / SABOTAR
	TALK,        // Falar com NPC / COLETAR INFORMAÇÕES

	ESCORT,      // Escoltar NPC até posição
	DEFEND,      // Proteger algo ou alguém
	CAPTURE,     // Capturar sem matar
	SURVIVE,      // Sobreviver por X segundos
	USE_ON,
};

enum MissionRewardType : uint8_t {
	XP,
	ITEM,
	ATTR_POINTS,
	SKILL_TRIES,
	PERK,
	KVE,
	SPELL 
};

struct MissionObjective {
	MissionObjectiveType type;
	std::string param;
	std::string description;
	uint32_t amount = 1;
	uint16_t itemId = 0;
};

struct MissionReward {
	MissionRewardType type;
	std::string name;
	std::string extraValue;
	uint32_t value = 0;
	std::string description; // Nova descrição amigável
	
};

struct MissionStage {
	uint32_t id;
	std::string name;

	std::vector<MissionObjective> objectives;
	std::vector<MissionReward> rewards;

	std::vector<std::pair<std::string, std::string>> kvRequirements;
	std::unordered_map<std::string, std::string> requirements; // Novos requisitos genericos
};

struct Mission {
	uint32_t id;
	std::string name;
	std::string description;
	bool repeatable = false;

	std::vector<MissionStage> stages;
};

class MissionManager final {
public:
	MissionManager() = default;

	MissionManager(const MissionManager&) = delete;
	MissionManager& operator=(const MissionManager&) = delete;

	static MissionManager &getInstance();
	static void init(lua_State* L);
	
	bool loadFromXML();
	const Mission* getMissionById(uint32_t id) const;
	const std::vector<Mission>& getMissions() const { return missions; }

	void setActiveMissionId(Player& player, uint32_t missionId) const;
	void setActiveMissionStage(Player& player, uint32_t stage) const;
	
	void checkStageCompletion(Player& player) const; 
	uint32_t getActiveMissionId(Player& player) const;
	uint32_t getActiveMissionStageIndex(Player& player) const;
	const MissionStage* getActiveMissionStage(Player& player) const;
	void applyRewards(Player& player, const MissionStage& stage) const;
	
	bool canStartStage(Player& player, uint32_t missionId, uint32_t stageId) const;
	bool hasCompletedAllObjectives(Player& player, const MissionStage& stage, bool removeCollectItems) const;

	std::string getStageRequirementsStatus(Player& player, uint32_t missionId, uint32_t stageId) const;
	std::string formatRequirementsForStage(Player& player, uint32_t missionId, uint32_t stageId) const;
	std::string getPlayerMissionStatus(Player& player) const;
	std::string getStageProgress(Player& player) const;

	void onCollectItem(Player& player, uint16_t itemId, uint32_t amount) const;
	void onKill(Player& player, const std::string& monsterName) const;
	void onUseItem(Player& player, uint16_t itemId) const;
	void onUseOnTarget(Player &player, uint16_t itemId, const std::string &targetName) const;
	void completeTalkObjective(Player &player, const std::string &npcName) const;

protected:
	bool loaded = false;

private:
	/* void loadMissionList(); */
	void loadMissionFile(const std::string& filepath);

	std::vector<Mission> missions;
};

constexpr auto g_missionManager = MissionManager::getInstance;
