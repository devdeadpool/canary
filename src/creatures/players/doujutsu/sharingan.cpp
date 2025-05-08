#include "creatures/players/doujutsu/sharingan.hpp"
#include "creatures/creatures_definitions.hpp"
#include "game/game.hpp"
#include "creatures/combat/condition.hpp"

Sharingan &Sharingan::getInstance() {
	static Sharingan instance;
	return instance;
}

const std::unordered_map<SharinganStage_t, SharinganStageData> sharinganStageData = {
	{ SHARINGAN_NONE_STAGE, {} },
	{ SHARINGAN_FIRST_STAGE, { { { SKILL_BUKIJUTSU, 5 }, { SKILL_TAIJUTSU, 5 }, { SKILL_CRITICAL_HIT_CHANCE, 10 } } } },
	{ SHARINGAN_SECOND_STAGE, { { { SKILL_NINJUTSU, 20 }, { SKILL_GENJUTSU, 15 } } } },
	{ SHARINGAN_THIRD_STAGE, { { { SKILL_NINJUTSU, 20 }, { SKILL_GENJUTSU, 15 }, { SKILL_CRITICAL_HIT_CHANCE, 20 } } } },
};

const SharinganStageData &Sharingan::getStageData(SharinganStage_t stage) const {
	static const SharinganStageData empty;
	auto it = sharinganStageData.find(stage);
	return it != sharinganStageData.end() ? it->second : empty;
}

std::string Sharingan::getStageName(SharinganStage_t stage) const {
	switch (stage) {
		case SHARINGAN_FIRST_STAGE:
			return "Sharingan (1 Tomoe)";
		case SHARINGAN_SECOND_STAGE:
			return "Sharingan (2 Tomoe)";
		case SHARINGAN_THIRD_STAGE:
			return "Sharingan (3 Tomoe)";
		default:
			return "";
	}
}

SharinganStage_t Sharingan::getStage(Player* player) const {
	int32_t stageValue = player->kv()->get("sharingan_stage").value_or(0);
	return static_cast<SharinganStage_t>(stageValue);
}

void Sharingan::setStage(Player* player, SharinganStage_t stage) const {
	player->kv()->set("sharingan_stage", static_cast<int32_t>(stage));
}

bool Sharingan::isActive(Player* player) const {
	return player->kv()->get("sharingan_active").value_or(false);
}

void Sharingan::setActive(Player* player, bool active) const {
	player->kv()->set("sharingan_active", active);
}

void Sharingan::toggle(Player* player) {
	if (!player || player->isRemoved()) {
		return;
	}

	SharinganStage_t stage = getStage(player);
	if (stage == SHARINGAN_NONE_STAGE) {
		player->sendTextMessage(MESSAGE_LOOK, "Você ainda não despertou o Sharingan.");
		return;
	}

	const auto &data = getStageData(stage);

	if (isActive(player)) {
		setActive(player, false);
		for (const auto &[skill, modifier] : data.skillModifiers) {
			player->setVarSkill(skill, -modifier);
		}
		player->sendSkills();
		player->sendTextMessage(MESSAGE_LOOK, "Kai!");
		return;
	}

	setActive(player, true);
	player->sendTextMessage(MESSAGE_LOOK, "Sharingan ativado!");
	g_game().addMagicEffect(player->getPosition(), CONST_ME_MAGIC_RED);

	for (const auto &[skill, modifier] : data.skillModifiers) {
		player->setVarSkill(skill, modifier);
	}
	player->sendSkills();
}
