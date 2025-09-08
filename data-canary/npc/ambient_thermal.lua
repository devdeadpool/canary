-- data/scripts/npcs/npc_ambient_yamato.lua
local npcName = "Yamato"
local npcType = Game.createNpcType(npcName)

local npcConfig = {
	name = npcName,
	description = "a silent observer.",
	health = 100,
	maxHealth = 100,
	walkInterval = 2000,
	walkRadius = 2,
	outfit = {
		lookType = 132, -- Altere conforme o sprite desejado
		lookHead = 0,
		lookBody = 95,
		lookLegs = 58,
		lookFeet = 79,
		lookAddons = 0
	},
	voices = {
        interval = 8000,
        chance = 30,
        {text = "As águas termais curam mais do que apenas o corpo."},
        {text = "Deixe suas preocupações se dissolverem na água quente."},
        {text = "Respire fundo... Sinta a paz da fonte."},
        {text = "Nada como um bom descanso após um longo treino."},
        {text = "O vapor da fonte renova o chakra e a alma."},
        {text = "Silêncio... até a mente precisa relaxar."},
        {text = "Mergulhe... e deixe que a natureza cuide do resto."},
        {text = "Aqui, até o tempo parece parar."}
    },
	flags = {
		floorchange = false
	}
}

npcType:register(npcConfig)
