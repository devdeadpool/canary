#include "creatures/players/status/player_attributes.hpp"
#include "creatures/players/player.hpp"

#include <vector>
#include <cstdint>
#include <algorithm> // std::max
#include <cmath> // se precisar de floor/ceil, etc.

// -------------------------------------------------
// Definição das faixas de custo (tiers)
//
// Cada tier diz: "Até X pontos de atributo, cada ponto custa Y pontos de status"
//
// Exemplos abaixo:
//  - Atributo <= 5  => custo = 1
//  - Atributo <= 10 => custo = 2
//  - Atributo <= 15 => custo = 3
//  - Atributo <= 20 => custo = 4
//  - Atributo <= 25 => custo = 5
//  - Atributo <= 30 => custo = 6
//  - Atributo <= 35 => custo = 7
//  - Atributo <= 40 => custo = 8
//  - Atributo <= 45 => custo = 9
//  - Atributo <= 50 => custo = 10
//
// Sinta-se livre para aumentar a lista ou mudar valores caso precise.
struct CostTier {
	uint32_t maxAttribute; // Até qual valor de atributo vale esse custo
	uint32_t costPerPoint; // Custo de cada ponto nessa faixa
};

// Lista de tiers. Se quiser continuar acima de 50, é só adicionar outro com maxAttribute grande.
static std::vector<CostTier> costTiers = {
	{ 7, 1 },
	{ 14, 2 },
	{ 21, 3 },
	{ 28, 4 },
	{ 35, 5 },
	{ 42, 6 },
	{ 49, 7 },
	{ 56, 8 },
	{ 64, 9 },
	{ 71, 10 },
};

// -------------------------------------------------
// Função auxiliar para obter o custo de UM ponto específico
// (Ex.: se o player estiver indo para level=6 de atributo, retorna 2, etc.)
static uint32_t getCostForSinglePoint(uint32_t level) {
	for (const auto &tier : costTiers) {
		if (level <= tier.maxAttribute) {
			return tier.costPerPoint;
		}
	}
	// Se passou do último tier, você pode retornar o custo do último ou criar outro tier "infinito"
	// Por segurança, retornaremos o custo do último tier
	if (!costTiers.empty()) {
		return costTiers.back().costPerPoint;
	}
	// Caso não exista costTiers, fallback
	return 1;
}

// -------------------------------------------------
// Calcula quanto custa aumentar o atributo de oldValue+1 até newValue inclusive.
// Ex.: se oldValue=4 e newValue=7, calculamos custo(5) + custo(6) + custo(7).
static uint32_t calculateIncrementalCost(uint32_t oldValue, uint32_t newValue) {
	if (newValue <= oldValue) {
		return 0; // não gastou nada se não subiu
	}

	uint32_t totalCost = 0;
	for (uint32_t level = oldValue + 1; level <= newValue; level++) {
		totalCost += getCostForSinglePoint(level);
	}
	return totalCost;
}

// -------------------------------------------------
// Função para calcular o "custo total" de ter chegado até 'value'.
// Usada no reset para devolver tudo que foi gasto até aquele valor.
static uint32_t getTotalCostForValue(uint32_t value) {
	// Se você considera que "ir de 0 para 1" deve cobrar,
	// calculateIncrementalCost(0, X) soma custo(1) + ... + custo(X).
	return calculateIncrementalCost(0, value);
}

// -------------------------------------------------
// Classe PlayerAttributes - Implementação

PlayerAttributes::PlayerAttributes(Player &player) :
	m_player(player) {
	// Se necessário, inicialize aqui
}

void PlayerAttributes::setStatusAttribute(PlayerStatus status, int value) {
	// Exemplo: "value" é quanto queremos acrescentar (pode ser 1, 2, etc.)
	uint32_t oldValue = attributes[static_cast<int>(status)];
	uint32_t newValue = oldValue + value;
	if (newValue < oldValue) {
		// Proteção contra overflow ou contra valores negativos
		return;
	}

	// Custo incremental de oldValue -> newValue
	uint32_t costNow = calculateIncrementalCost(oldValue, newValue);

	// Se o jogador não tiver pontos de status suficientes, aborta (ou faz parcial).
	if (costNow > static_cast<uint32_t>(statusPoints)) {
		m_player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("Not enough points! Need {}, have {}.", costNow, statusPoints));
		return;
	}

	// Desconta do statusPoints
	statusPoints -= costNow;

	// Seta o novo valor do atributo
	attributes[static_cast<int>(status)] = newValue;

	// Manda atualização pro cliente, se desejar
	m_player.sendPlayerAttributes();
	updateDerivedStats();
}

// Remove pontos de status (caso queira penalizar ou tirar statusPoints)
void PlayerAttributes::removeStatusPoints(int value) {
	statusPoints = std::max(0, statusPoints - value);
}

int PlayerAttributes::getStatusAttribute(PlayerStatus status) const {
	return attributes[static_cast<int>(status)];
}

// Adiciona pontos de status ao player (ex.: quando ele upa level)
void PlayerAttributes::addStatusPoints(int value) {
	statusPoints += value;
}

// Seta diretamente (se precisar) o valor de um atributo (cuidado: não desconta custo).
void PlayerAttributes::setBaseAttribute(PlayerStatus status, int value) {
	attributes[static_cast<int>(status)] = value;
}

// Retorna pontos de status não gastos
int PlayerAttributes::getStatusPoints() const {
	return statusPoints;
}

/* void PlayerAttributes::resetStatusAttributes() {
	uint32_t total = statusPoints;

	for (int i = 0; i < static_cast<int>(PlayerStatus::LAST); i++) {
		uint32_t value = attributes[i];
		uint32_t base = m_player.vocation->baseAttributes[i];

		if (value > base) {
			total += calculateIncrementalCost(base, value);
		}

		attributes[i] = base;
	}

	statusPoints = total;

	m_player.sendPlayerAttributes();
	updateDerivedStats();

	m_player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("All attributes reset! You now have {} status points.", statusPoints));
} */
// --------------------------------------------
// Funções de highestLevel (para controlar se já ganhou pontos de level anterior)

int PlayerAttributes::getHighestLevel() const {
	return highestLevel;
}

void PlayerAttributes::setHighestLevel(int level) {
	highestLevel = level;
}

// Quando o player upa, atualiza statusPoints
void PlayerAttributes::updatePoints(uint32_t oldLevel, uint32_t newLevel) {
	int hLevel = getHighestLevel();
	if (newLevel <= static_cast<uint32_t>(hLevel)) {
		// se o jogador já atingiu ou ultrapassou esse nível antes, não ganha de novo
		return;
	}

	setHighestLevel(newLevel);

	// Quantos níveis foram ganhos
	uint32_t qtdLvl = newLevel - oldLevel;

	// Pega quantos pontos por level a vocação desse player fornece
	uint32_t pointsToAdd = m_player.getPointsPerLevel() * qtdLvl;

	// Adiciona esses pontos
	addStatusPoints(pointsToAdd);

	// Envia update pro cliente
	m_player.sendPlayerAttributes();
	updateDerivedStats();

	m_player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("You gained {} status points!", pointsToAdd));
}

// --------------------------------------------
// Se quiser checar se o player PODE gastar 1 ponto naquele atributo, utilize a tier:
bool PlayerAttributes::canSpendStatusPoint(PlayerStatus status) const {
	// Quanto custaria subir +1 ponto nesse atributo
	// Ex.: se atributo = 5, então o próximo ponto é "o 6", custaria 2 (no tier).
	int attrValue = getStatusAttribute(status);
	uint32_t nextPointCost = getCostForSinglePoint(attrValue + 1);

	return statusPoints >= static_cast<int>(nextPointCost);
}

// Quanto custaria subir +1 ponto nesse atributo
int PlayerAttributes::getStatusPointCost(PlayerStatus status) const {
	int attrValue = getStatusAttribute(status);
	// Para descobrir o custo do "próximo nível"
	uint32_t nextPointCost = getCostForSinglePoint(attrValue + 1);
	return static_cast<int>(nextPointCost);
}

void PlayerAttributes::updateDerivedStats() {
	const int oldHealthMax = m_player.healthMax;
	const int oldManaMax = m_player.manaMax;

	m_player.healthMax = m_player.getMaxHealth();
	m_player.manaMax = m_player.getMaxMana();

	int healthGain = m_player.healthMax - oldHealthMax;
	int manaGain = m_player.manaMax - oldManaMax;

	if (healthGain > 0) {
		m_player.health = std::min(m_player.health + healthGain, m_player.healthMax);
	} else {
		m_player.health = std::min(m_player.health, m_player.healthMax);
	}

	if (manaGain > 0) {
		m_player.mana = std::min(m_player.mana + manaGain, m_player.manaMax);
	} else {
		m_player.mana = std::min(m_player.mana, m_player.manaMax);
	}

	m_player.getCombatStats().calculateFromPlayer(&m_player);

	m_player.sendHealthBarUpdate();
	m_player.sendStats();
}

void PlayerAttributes::applyBaseAttributesFromVocation() {
	for (int i = 0; i < static_cast<int>(PlayerStatus::LAST); ++i) {
		/* uint16_t base = m_player.vocation->baseAttributes[i];
		setBaseAttribute(static_cast<PlayerStatus>(i), base); */
	}
	updateDerivedStats();
	saveToDatabase();
	m_player.sendPlayerAttributes();
	m_player.sendStats();
}

void PlayerAttributes::saveToDatabase() {
	std::ostringstream query;
	query << "UPDATE `player_attributes` SET "
		  << "`strength` = " << getStatusAttribute(PlayerStatus::STRENGTH) << ", "
		  << "`agility` = " << getStatusAttribute(PlayerStatus::AGILITY) << ", "
		  << "`intelligence` = " << getStatusAttribute(PlayerStatus::INTELIGGENCE) << ", "
		  << "`energy` = " << getStatusAttribute(PlayerStatus::ENERGY) << ", "
		  << "`focus` = " << getStatusAttribute(PlayerStatus::FOCUS) << ", "
		  << "`perception` = " << getStatusAttribute(PlayerStatus::PERCEPTION) << ", "
		  << "`determination` = " << getStatusAttribute(PlayerStatus::DETERMINATION) << ", "
		  << "`highest_level` = " << getHighestLevel() << ", "
		  << "`status_points` = " << getStatusPoints()
		  << " WHERE `player_id` = " << m_player.getGUID();
/* 
	if (!Database::getInstance().executeQuery(query.str())) {
		g_logger().warn("[saveToDatabase] Failed to save attributes for player {}", m_player.getName());
	} */
}
