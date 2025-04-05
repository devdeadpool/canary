#pragma once

#include "creatures/players/status/player_status.hpp"

class Player;

class PlayerAttributes {
public:
	explicit PlayerAttributes(Player &player);

	void setStatusAttribute(PlayerStatus status, int value);
	int getStatusAttribute(PlayerStatus status) const;
	void addStatusPoints(int value);
	int getStatusPoints() const;
	void resetStatusAttributes();
	void setHighestLevel(int level);
	int getHighestLevel() const;
	void updatePoints(uint32_t oldLevel, uint32_t newLevel);
	bool canSpendStatusPoint(PlayerStatus status) const;
	int getStatusPointCost(PlayerStatus status) const;
	void removeStatusPoints(int value);
	void setBaseAttribute(PlayerStatus status, int value);
	void updateDerivedStats();
	void applyBaseAttributesFromVocation();
	void saveToDatabase();

private:
	Player &m_player;
	int attributes[static_cast<int>(PlayerStatus::LAST)] = { 0 }; // Array para armazenar os valores
	// std::array<int, static_cast<int>(PlayerStatus::LAST)> attributes;
	int statusPoints = 0;
	int highestLevel = 1;
};
