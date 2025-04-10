// combat_stats.cpp
#include "creatures/combat/combat_stats.hpp"
#include "creatures/players/player.hpp"
#include "creatures/monsters/monster.hpp"
#include "creatures/players/status/player_attributes.hpp"
#include "creatures/monsters/monsters.hpp"
#include "lua/lua_definitions.hpp"

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
	if (!monster)
		return;

	const std::shared_ptr<MonsterType>& monsterType = monster->getMonsterType();
	if (!monsterType)
		return;

	const auto& info = monsterType->info;

	// Função auxiliar para calcular um valor base
	auto autoScale = [monster]() -> int32_t {
		return std::max<int32_t>(10, monster->getMaxHealth() / 100);
	};


	if (!monsterType) {
		g_logger().warn("❌ MonsterType nulo para o monstro {}", monster->getName());
		return;
	}

	// Atribuição dos stats com fallback automático
	stats[SHINOBISTAT_ATK]    = (info.atk    >= -1) ? info.atk    : autoScale();
	stats[SHINOBISTAT_DEF]    = (info.def    >= -1) ? info.def    : std::max(0, autoScale() - 5);
	stats[SHINOBISTAT_SP_ATK] = (info.spAtk  >= -1) ? info.spAtk  : autoScale() + 10;
	stats[SHINOBISTAT_SP_DEF] = (info.spDef  >= -1) ? info.spDef  : autoScale() + 5;
}

const CombatStats& CombatStats::empty() {
	static CombatStats emptyInstance;
	return emptyInstance;
}


void CombatStats::logDebugPointer(const std::string& context, const std::shared_ptr<Creature>& creature) const {
	if (!creature) return;

	g_logger().info("🔬 {} → Pointer CombatStats de {}: {}", context, creature->getName(), fmt::ptr(this));
}