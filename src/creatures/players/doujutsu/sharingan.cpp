#include "creatures/players/doujutsu/sharingan.hpp"
#include "creatures/creatures_definitions.hpp"
#include "game/game.hpp"
#include "creatures/combat/condition.hpp"
#include "creatures/players/vocations/vocation.hpp"
#include "creatures/combat/spells.hpp"


Sharingan &Sharingan::getInstance() {
	static Sharingan instance;
	return instance;
}

const std::unordered_map<SharinganStage_t, SharinganStageData> sharinganStageData = {
	{ SHARINGAN_NONE_STAGE, {} },

	{ SHARINGAN_FIRST_STAGE, { { { SKILL_BUKIJUTSU, 5 }, { SKILL_TAIJUTSU, 5 }, { SKILL_CRITICAL_HIT_CHANCE, 10 } } } },

	{ SHARINGAN_SECOND_STAGE, { { { SKILL_NINJUTSU, 20 }, { SKILL_GENJUTSU, 15 } } } },

	{ SHARINGAN_THIRD_STAGE, { { { SKILL_NINJUTSU, 20 }, { SKILL_GENJUTSU, 15 }, { SKILL_CRITICAL_HIT_CHANCE, 20 } } } }
};

const std::unordered_map<SharinganStage_t, SharinganEvolutionInfo> sharinganEvolutionMap = {
	{ SHARINGAN_FIRST_STAGE, { .requiredSeconds = 0, // 🔥 Usar Sharingan por 5 minutos
	                           .requiredSkills = {},
	                           .requiredFlags = { "sharingan_near_death" }, // ❤️ Sobreviveu com <15% de HP
	                           .clearFlagsOnEvolve = { "sharingan_near_death" },
	                           .message = "Você despertou o primeiro tomoe do Sharingan!" } },
	{ SHARINGAN_SECOND_STAGE, { .requiredSeconds = 0, // 🔥 Usar Sharingan por 15 minutos
	                            .requiredSkills = { { SKILL_GENJUTSU, 50 } },
	                            .requiredFlags = {
									"sharingan_trauma_party_death", // ✅ Aliado da party morreu perto
									"sharingan_near_death", // ❤️ HP muito baixo
									"sharingan_chakra_low" // 🔵 Chakra < 10%
								},
	                            .clearFlagsOnEvolve = { "sharingan_trauma_party_death", "sharingan_near_death", "sharingan_chakra_low" },
	                            .message = "Você despertou o segundo tomoe do Sharingan!" } },
	{ SHARINGAN_THIRD_STAGE, { .requiredSeconds = 0, // 🔥 Usar Sharingan por 30 minutos
	                           .requiredSkills = { { SKILL_GENJUTSU, 70 }, { SKILL_NINJUTSU, 60 } },
	                           .requiredFlags = {
								   "sharingan_near_death", // 💔 Morte marcante
								   "sharingan_trauma_party_death" // 😡 Emoção extrema
							   },
	                           .clearFlagsOnEvolve = { "sharingan_near_death", "sharingan_trauma_party_death" },
	                           .message = "Você despertou o terceiro tomoe do Sharingan!" } },
	{ SHARINGAN_MANGEKYOU, { .requiredSeconds = 3600, .requiredSkills = { { SKILL_GENJUTSU, 90 }, { SKILL_NINJUTSU, 80 } }, .requiredFlags = { "sharingan_killed_friend" }, .clearFlagsOnEvolve = { "sharingan_killed_friend" }, .message = "Você despertou o Mangekyou Sharingan!" } }
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
		updateEyeItem(player);
		return;
	}

	setActive(player, true);
	player->sendTextMessage(MESSAGE_LOOK, "Sharingan ativado!");
	g_game().addMagicEffect(player->getPosition(), CONST_ME_MAGIC_RED);

	for (const auto &[skill, modifier] : data.skillModifiers) {
		player->setVarSkill(skill, modifier);
	}
	player->sendSkills();
	updateEyeItem(player);
}

void Sharingan::learn(Player* player) const {
	if (getStage(player) == SHARINGAN_NONE_STAGE) {
		setStage(player, SHARINGAN_FIRST_STAGE);
		setUsageSeconds(player, 0);
		setActive(player, false);
		player->sendTextMessage(MESSAGE_EVENT_ADVANCE, "Você despertou o poder do Sharingan!");
	}
}

void Sharingan::tryEvolve(Player* player) const {
	const SharinganStage_t currentStage = getStage(player);
	const SharinganStage_t nextStage = static_cast<SharinganStage_t>(static_cast<int>(currentStage) + 1);

	// Busca os requisitos do PRÓXIMO estágio
	auto it = sharinganEvolutionMap.find(nextStage);
	if (it == sharinganEvolutionMap.end()) {
		return;
	}

	const auto &info = it->second;

	// Checa tempo acumulado
	const int32_t seconds = getUsageSeconds(player);
	if (seconds < info.requiredSeconds) {
		return;
	}

	// Checa pré-requisitos de skills
	for (const auto &[skill, requiredLevel] : info.requiredSkills) {
		if (player->getSkillLevel(skill) < requiredLevel) {
			return;
		}
	}

	// Checa se pelo menos uma flag obrigatória está presente (se houver)
	if (!info.requiredFlags.empty()) {
		bool hasAnyFlag = false;
		for (const std::string &flag : info.requiredFlags) {
			if (player->kv()->get(flag).value_or(false)) {
				hasAnyFlag = true;
				break;
			}
		}
		if (!hasAnyFlag) {
			return;
		}
	}

	// Realiza a evolução
	setStage(player, nextStage);
	player->sendTextMessage(MESSAGE_EVENT_ADVANCE, info.message);

	// Reseta tempo de uso
	player->kv()->set("sharingan_seconds", 0);

	// Limpa todas as flags que devem ser apagadas ao evoluir
	for (const std::string &flag : info.clearFlagsOnEvolve) {
		player->kv()->remove(flag);
	}
}

int32_t Sharingan::getUsageSeconds(Player* player) const {
	return player->kv()->get("sharingan_usage_seconds").value_or(0);
}

void Sharingan::setUsageSeconds(Player* player, int32_t seconds) const {
	player->kv()->set("sharingan_usage_seconds", seconds);
}

void Sharingan::addUsageSeconds(Player* player, int32_t seconds) const {
	int32_t current = getUsageSeconds(player);
	setUsageSeconds(player, current + seconds);
}

void Sharingan::resetSharinganProgress(Player* player) const {
	player->kv()->remove("sharingan_stage");
	player->kv()->remove("sharingan_active");
	player->kv()->remove("sharingan_usage_seconds");
}

std::string Sharingan::getStageProgressInfo(SharinganStage_t stage, int32_t seconds) const {
	switch (stage) {
		case SHARINGAN_FIRST_STAGE:
			return fmt::format("Faltam {}s para evoluir para o segundo tomoe.", std::max(0, 300 - seconds));
		case SHARINGAN_SECOND_STAGE:
			return fmt::format("Faltam {}s e Genjutsu 50 para o terceiro tomoe.", std::max(0, 900 - seconds));
		case SHARINGAN_THIRD_STAGE:
			return "Seu Sharingan esta completo!";
		default:
			return "";
	}
}

void Sharingan::updateEyeItem(Player* player) const {
	if (!player) {
		return;
	}

	// Remove o olho atual
	if (const auto &currentEye = player->getInventoryItem(CONST_SLOT_EYE)) {
		g_game().internalRemoveItem(currentEye);
	}

	// Decide o novo item de olho
	uint16_t newEyeId = 0;

	if (isActive(player)) {
		switch (getStage(player)) {
			case SHARINGAN_FIRST_STAGE:
				newEyeId = ITEM_SHARINGAN_FIRST_STAGE;
				break;
			case SHARINGAN_SECOND_STAGE:
				newEyeId = ITEM_SHARINGAN_SECOND_STAGE;
				break;
			case SHARINGAN_THIRD_STAGE:
				newEyeId = ITEM_SHARINGAN_THIRD_STAGE;
				break;
			case SHARINGAN_MANGEKYOU:
				newEyeId = ITEM_SHARINGAN_MANGEKYOU;
				break;
			default:
				break;
		}
	} else {
		newEyeId = player->getVocation()->getNormalEye();
	}

	if (newEyeId != 0) {
		auto item = Item::CreateItem(newEyeId);
		g_game().internalAddItem(player->getPlayer(), item, CONST_SLOT_EYE, FLAG_NOLIMIT);
	}
}

int32_t Sharingan::getJutsuSlots(Player* player) const {
	return player->kv()->get("sharingan_jutsu_slots").value_or(1);
}

void Sharingan::setJutsuSlots(Player* player, int32_t slots) const {
	player->kv()->set("sharingan_jutsu_slots", std::min(slots, 5));
}

void Sharingan::addJutsuSlots(Player* player, int32_t amount) const {
	int32_t current = getJutsuSlots(player);
	setJutsuSlots(player, current + amount);
}

const std::shared_ptr<KV>& Sharingan::getCopiedKV(Player* player) const {
	if (!m_copiedKV || m_cachedPlayer != player) {
		m_cachedPlayer = player;
		m_copiedKV = player->kv()->scoped("sharingan")->scoped("copied");
	}
	return m_copiedKV;
}

bool Sharingan::hasCopiedJutsu(Player* player, const std::string& jutsuName) const {
	return getCopiedKV(player)->get(jutsuName).value_or(false);
}

bool Sharingan::addCopiedJutsu(Player* player, const std::string& jutsuName) const {
	if (hasCopiedJutsu(player, jutsuName))
		return false;

	getCopiedKV(player)->set(jutsuName, true);
	return true;
}

std::vector<std::string> Sharingan::getCopiedJutsus(Player* player) const {
	const auto& keys = getCopiedKV(player)->keys();
	return std::vector<std::string>(keys.begin(), keys.end());
}

void Sharingan::tryCopyJutsu(Player* player, const std::shared_ptr<Spell>& spell) const {
	if (!player || !spell || !isActive(player)) return;
	if (getStage(player) < SHARINGAN_MANGEKYOU) return;

	const auto instantSpell = std::dynamic_pointer_cast<InstantSpell>(spell);
	if (!instantSpell || !instantSpell->isInstant()) return;

	const std::string& spellName = instantSpell->getName();
	if (hasCopiedJutsu(player, spellName)) return;

	const int32_t unlockedSlots = getJutsuSlots(player);
	std::vector<std::string> copiedList = getCopiedJutsus(player);

	if (static_cast<int32_t>(copiedList.size()) >= unlockedSlots) {
		player->sendTextMessage(MESSAGE_LOGIN, "Você não tem slots suficientes para copiar mais jutsus.");
		return;
	}

	addCopiedJutsu(player, spellName);
	player->sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("Você copiou o jutsu: {}!", spellName));
	g_game().addMagicEffect(player->getPosition(), CONST_ME_MAGIC_BLUE);
}
