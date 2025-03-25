#include "creatures/players/status/player_attributes.hpp"
#include "creatures/players/player.hpp"

PlayerAttributes::PlayerAttributes(Player &player) :
	m_player(player) { }

void PlayerAttributes::setStatusAttribute(PlayerStatus status, int value) {
	int currentAttribute = getStatusAttribute(status);
	int totalCost = 0;

	// Adiciona os pontos um por um, recalculando o custo progressivo a cada incremento
	for (int i = 0; i < value; i++) {
		int cost = getStatusPointCost(static_cast<PlayerStatus>(status));
		if (statusPoints < cost) {
			break; // Para se não houver pontos suficientes
		}
		statusPoints -= cost; // Deduz o custo ponto a ponto
		currentAttribute++; // Aumenta um ponto no atributo
		totalCost += cost; // Soma o custo total gasto
	}

	attributes[static_cast<int>(status)] = currentAttribute; // Define o novo valor corretamente

	// Log de depuração para checar os cálculos
	g_logger().info("[setAttribute] Attribute {} increased to {} (Total Cost: {})", static_cast<int>(status), currentAttribute, totalCost);
}

void PlayerAttributes::removeStatusPoints(int value) {
	statusPoints = std::max(0, statusPoints - value); // Evita valores negativos
}

// Retorna o valor de um atributo específico
int PlayerAttributes::getStatusAttribute(PlayerStatus status) const {
	return attributes[static_cast<int>(status)];
}

// Adiciona pontos de status
void PlayerAttributes::addStatusPoints(int value) {
	statusPoints += value;
}

void PlayerAttributes::setBaseAttribute(PlayerStatus status, int value) {
	attributes[static_cast<int>(status)] = value;
}

// Retorna os pontos disponíveis
int PlayerAttributes::getStatusPoints() const {
	return statusPoints;
}

// Reseta os atributos e devolve os pontos
void PlayerAttributes::resetStatusAttributes() {
	int total = statusPoints;
	for (int i = 0; i < static_cast<int>(PlayerStatus::LAST); i++) {
		total += attributes[i];
		attributes[i] = 0;
	}
	statusPoints = total;
}

int PlayerAttributes::getHighestLevel() const {
	return highestLevel;
}

void PlayerAttributes::setHighestLevel(int level) {
	highestLevel = level;
}

void PlayerAttributes::updatePoints(uint32_t oldLevel, uint32_t newLevel) {
	int highestLevel = getHighestLevel();

	if (newLevel <= highestLevel) {
		// Se o jogador já atingiu esse nível antes, não ganha pontos novamente
		return;
	}

	// Atualiza o novo nível máximo alcançado
	setHighestLevel(newLevel);

	// Calcula quantos níveis foram ganhos
	uint32_t qtdLvl = newLevel - oldLevel;

	// Obtém quantos pontos essa vocação ganha por nível
	uint32_t pointsToAdd = m_player.getPointsPerLevel() * qtdLvl;

	// Adiciona os pontos ao jogador
	addStatusPoints(pointsToAdd);

	m_player.sendTextMessage(MESSAGE_EVENT_ADVANCE, fmt::format("You gained {} status points!", pointsToAdd));
}

bool PlayerAttributes::canSpendStatusPoint(PlayerStatus status) const {
	int cost = getStatusPointCost(status);
	return statusPoints >= cost;
}

int PlayerAttributes::getStatusPointCost(PlayerStatus status) const {
	int attributeValue = getStatusAttribute(status);
	return std::ceil(std::log2(attributeValue + 2)) + 1;
}
