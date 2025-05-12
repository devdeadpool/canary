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

struct SharinganEvolutionInfo {
	int32_t requiredSeconds;
	std::unordered_map<skills_t, uint32_t> requiredSkills;
	std::vector<std::string> requiredFlags;
	std::vector<std::string> clearFlagsOnEvolve;
	std::string message;
};

// Struct com os bônus por estágio
struct SharinganStageData {
    std::unordered_map<skills_t, int32_t> skillModifiers;
};

class Sharingan final {
public:
  static Sharingan& getInstance();
  const SharinganStageData& getStageData(SharinganStage_t stage) const;
  std::string getStageName(SharinganStage_t stage) const;


	void toggle(Player* player);
	bool isActive(Player* player) const;
	void setActive(Player* player, bool active) const;
	SharinganStage_t getStage(Player* player) const;
	void setStage(Player* player, SharinganStage_t stage) const;

        // Aprendizado e evolução
    void learn(Player* player) const;
    void tryEvolve(Player* player) const;

    // Tempo de uso
    int32_t getUsageSeconds(Player* player) const;
    void setUsageSeconds(Player* player, int32_t seconds) const;
    void addUsageSeconds(Player* player, int32_t seconds) const;

    // Reset total (opcional)
    void resetSharinganProgress(Player* player) const;
    std::string getStageProgressInfo(SharinganStage_t stage, int32_t seconds) const;

    void updateEyeItem(Player* player) const;

private:
    Sharingan() = default;
    Sharingan(const Sharingan&) = delete;         
    Sharingan& operator=(const Sharingan&) = delete;
};

// Instância global
constexpr auto g_sharingan = Sharingan::getInstance;
extern const std::unordered_map<SharinganStage_t, SharinganEvolutionInfo> sharinganEvolutionMap;










