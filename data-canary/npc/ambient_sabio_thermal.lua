
-- data/scripts/npcs/npc_ambient_yamato.lua
local npcName = "Sabio Enmei"
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
        {text = "A sabedoria cresce no silêncio... como as raízes na água."},
        {text = "Até os guerreiros mais fortes precisam descansar."},
        {text = "A fonte cura o corpo... o tempo cura a alma."},
        {text = "Já vi muitos verões... mas nenhum tão tranquilo quanto este."},
        {text = "Sente-se. Escute a água. Aprenda com ela."},
        {text = "A mente agitada não encontra paz nem na fonte mais pura."},
        {text = "Descansar não é fraqueza... é preparação para o próximo passo."}
    },
	flags = {
		floorchange = false
	},
}

npcType:register(npcConfig)
