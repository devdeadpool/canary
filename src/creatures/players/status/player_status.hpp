#pragma once

#ifndef USE_PRECOMPILED_HEADERS
    #include <cstdint>
#endif


enum class PlayerStatus : uint8_t {
    STRENGTH = 0, // Força
    AGILITY = 1, // Agilidade
    INTELIGGENCE = 2, // Inteligência
    ENERGY = 3,// Energia
    FOCUS = 4,// Concentração
    PERCEPTION = 5, // Percepção
    DETERMINATION = 6,// Determinação
    LAST // Sempre o último
};