local npcName = "Nin Iyashi"
local npcType = Game.createNpcType(npcName)
local npcConfig = {}

npcConfig.name = npcName
npcConfig.description = "medical-nin."
npcConfig.health = 100
npcConfig.maxHealth = 100
npcConfig.walkInterval = 2000
npcConfig.walkRadius = 3
npcConfig.outfit = {
    lookType = 848, -- Exemplo de outfit
    lookHead = 0,
    lookBody = 94,
    lookLegs = 76,
    lookFeet = 79,
    lookAddons = 0
}
npcConfig.voices = {
    interval = 10000,
    chance = 20,
    { text = "Precisa de cuidados médicos? Basta me pedir para curar você!" },
    { text = "Um ninja saudável é um ninja forte!" }
}
npcConfig.flags = { floorchange = false }

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
npcHandler:addModule(FocusModule:new(), npcConfig.name, true, true, true)

-- Função de cura progressiva (3 segundos)
local function progressiveHeal(player, npc, totalTime)
    local steps = 30 -- 3 segundos = 100ms por step
    local interval = totalTime / steps
    local hpStart, hpMax = player:getHealth(), player:getMaxHealth()
    local mpStart, mpMax = player:getMana(), player:getMaxMana()
    local chakraStart, chakraMax = player.getChakra and player:getChakra() or 0, player.getMaxChakra and player:getMaxChakra() or 0

    for i = 1, steps do
        addEvent(function()
            if not player or player:isRemoved() then return end
            local hpNow = hpStart + math.ceil((hpMax - hpStart) * (i / steps))
            local mpNow = mpStart + math.ceil((mpMax - mpStart) * (i / steps))
            local chakraNow = chakraStart > 0 and (chakraStart + math.ceil((chakraMax - chakraStart) * (i / steps))) or nil
            player:addHealth(hpNow - player:getHealth())
            player:addMana(mpNow - player:getMana())
            if chakraNow and player.getChakra and player.setChakra then
                player:setChakra(chakraNow)
            end
            player:getPosition():sendMagicEffect(13)
            if i == steps then
                npcHandler:say("Pronto! Você está totalmente curado.", npc, player)
            end
        end, math.floor(i * interval))
    end
end
-- Handler universal para qualquer frase com "curar", "cura", "heal", "recuperar" etc
npcType.onSay = function(npc, player, messageType, msg)
    local m = msg:lower()
    if m:find("curar") or m:find("cura") or m:find("hi") or m:find("oi") or m:find("ola") or m:find("hello")  or m:find("heal") or m:find("recuperar") or m:find("healing") then
        if player:getHealth() >= player:getMaxHealth()
            and player:getMana() >= player:getMaxMana()
            and (not player.getChakra or player:getChakra() >= player:getMaxChakra()) then
            npcHandler:say("Você já está totalmente saudável!", npc, player)
            return
        end
        npcHandler:say("Vou curar você agora, aguarde um momento...", npc, player)
        progressiveHeal(player, npc, 3000) -- 3 segundos
        return
    end
end

npcType.onAppear = function(npc, creature) npcHandler:onAppear(npc, creature) end
npcType.onDisappear = function(npc, creature) npcHandler:onDisappear(npc, creature) end
npcType.onMove = function(npc, creature, fromPosition, toPosition) npcHandler:onMove(npc, creature, fromPosition, toPosition) end
npcType.onThink = function(npc, interval) npcHandler:onThink(npc, interval) end

npcType:register(npcConfig)