// sharingan.hpp
#pragma once

#include "creatures/creatures_definitions.hpp"
#include "creatures/players/player.hpp"
#include <unordered_map>
#include <string>
#include <cstdint>

// Estágios do Sharingan
enum SharinganStage_t : uint8_t {
	SHARINGAN_NONE_STAGE = 0,
	SHARINGAN_FIRST_STAGE = 1,
	SHARINGAN_SECOND_STAGE = 2,
	SHARINGAN_THIRD_STAGE = 3,
	SHARINGAN_MANGEKYOU = 4,
	SHARINGAN_FUUMETSU_MANGEKYOU = 5,
};

// Struct com os bônus por estágio
struct SharinganStageData {
	std::unordered_map<skills_t, int32_t> skillModifiers;
};

// Classe central do sistema Sharingan
class Sharingan final {
public:
	static Sharingan &getInstance();
	const SharinganStageData &getStageData(SharinganStage_t stage) const;
	std::string getStageName(SharinganStage_t stage) const;

	void toggle(Player* player);
	bool isActive(Player* player) const;
	void setActive(Player* player, bool active) const;
	SharinganStage_t getStage(Player* player) const;
	void setStage(Player* player, SharinganStage_t stage) const;

private:
	Sharingan() = default;
	Sharingan(const Sharingan &) = delete;
	Sharingan &operator=(const Sharingan &) = delete;
};

// Instância global
constexpr auto g_sharingan = Sharingan::getInstance;
