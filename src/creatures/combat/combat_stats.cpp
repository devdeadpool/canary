// combat_stats.cpp
#include "creatures/combat/combat_stats.hpp"
#include "creatures/players/player.hpp"
#include "creatures/monsters/monster.hpp"
#include "creatures/players/status/player_attributes.hpp"

void CombatStats::reset() {
	stats.fill(0);
}

void CombatStats::calculateFromPlayer(Player* player) {
	const auto &attr = player->playerAttributes();
	const auto* skills = player->getSkills();

	int energy = attr.getStatusAttribute(PlayerStatus::ENERGY);
	int strength = attr.getStatusAttribute(PlayerStatus::STRENGTH);
	int resistance = skills[SKILL_RESISTANCE].level; // Agora usando a skill Resistance
	int intelligence = attr.getStatusAttribute(PlayerStatus::INTELIGGENCE);
	int agility = attr.getStatusAttribute(PlayerStatus::AGILITY);

	stats[SHINOBISTAT_STAMINA] = (energy * 4)
		+ (skills[SKILL_TAIJUTSU].level * 20)
		+ (skills[SKILL_BUKIJUTSU].level * 7);

	stats[SHINOBISTAT_ATK] = strength / 2;
	stats[SHINOBISTAT_DEF] = resistance / 2;
	stats[SHINOBISTAT_SP_ATK] = intelligence / 2;
	stats[SHINOBISTAT_SP_DEF] = resistance / 2;

	stats[SHINOBISTAT_POWER] = (energy * 200)
		+ (strength + intelligence + resistance) * 150
		+ (skills[SKILL_NINJUTSU].level + skills[SKILL_GENJUTSU].level + skills[SKILL_BUKIJUTSU].level + skills[SKILL_TAIJUTSU].level + agility
	      ) * 100;
}

void CombatStats::calculateFromMonster(Monster* monster) {
	// Placeholder para lógica de monstros
	// Exemplo fixo:
	stats[SHINOBISTAT_STAMINA] = 100;
	stats[SHINOBISTAT_ATK] = 10;
	stats[SHINOBISTAT_DEF] = 8;
	stats[SHINOBISTAT_SP_ATK] = 12;
	stats[SHINOBISTAT_SP_DEF] = 9;
	stats[SHINOBISTAT_POWER] = 500;
}
