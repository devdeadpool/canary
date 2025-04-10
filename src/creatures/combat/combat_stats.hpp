// combat_stats.hpp
#pragma once

#include <array>
#include <cstdint>
#include "creatures/creature.hpp"

class Player;
class Monster;
class Creature;

enum shinobiStat_t : uint8_t {
	SHINOBISTAT_STAMINA = 0,
	SHINOBISTAT_ATK,
	SHINOBISTAT_DEF,
	SHINOBISTAT_SP_ATK,
	SHINOBISTAT_SP_DEF,
	SHINOBISTAT_POWER,
	SHINOBISTAT_LAST = SHINOBISTAT_POWER
};

class CombatStats {
public:
	CombatStats() {
		reset();
	}

	void reset();
	void calculateFromPlayer(Player* player);
	void calculateFromMonster(Monster* monster);
	static const CombatStats& empty();
	void logDebugPointer(const std::string& context, const std::shared_ptr<Creature>& creature) const;

	int32_t get(shinobiStat_t stat) const {
		return stats[stat];
	}
	void set(shinobiStat_t stat, int32_t value) {
		stats[stat] = value;
	}

private:
	std::array<int32_t, SHINOBISTAT_LAST + 1> stats;
};
