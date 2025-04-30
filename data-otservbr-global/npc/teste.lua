local internalNpcName = "Mission Master"
local npcType = Game.createNpcType(internalNpcName)
local npcConfig = {}

npcConfig.name = internalNpcName
npcConfig.description = "A master who guides shinobis through difficult missions."

npcConfig.health = 100
npcConfig.maxHealth = 100
npcConfig.walkInterval = 2000
npcConfig.walkRadius = 2

npcConfig.outfit = {
    lookType = 130,
    lookHead = 19,
    lookBody = 50,
    lookLegs = 40,
    lookFeet = 76,
    lookAddons = 3,
}

npcConfig.flags = {
    floorchange = false,
}

local MISSION_ID = 11 -- ID da missão "Uchiha Trials"

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)

npcType.onThink = function(npc, interval) npcHandler:onThink(npc, interval) end
npcType.onAppear = function(npc, creature) npcHandler:onAppear(npc, creature) end
npcType.onDisappear = function(npc, creature) npcHandler:onDisappear(npc, creature) end
npcType.onMove = function(npc, creature, fromPos, toPos) npcHandler:onMove(npc, creature, fromPos, toPos) end
npcType.onSay = function(npc, creature, type, message) npcHandler:onSay(npc, creature, type, message) end
npcType.onCloseChannel = function(npc, creature) npcHandler:onCloseChannel(npc, creature) end

local function creatureSayCallback(npc, creature, type, message)
    local player = Player(creature)
    if not player or not npcHandler:checkInteraction(npc, creature) then
        return false
    end

    message = message:lower()

    if message:find("mission") then
        local activeMissionId = player:getActiveMissionId()

        if activeMissionId == 0 then
            if player:canStartStage(MISSION_ID) then
                npcHandler:say("I have a mission called 'Uchiha Trials'. If you wish to accept it, say {start}.", npc, creature)
            else
                local requirements = player:getStageRequirementsStatus(MISSION_ID)
                npcHandler:say("You do not meet the requirements to start the mission.\n\n" .. requirements, npc, creature)
            end
            return true

        elseif activeMissionId == MISSION_ID then
            if not player:checkStageCompletion() then
                local progress = player:getStageProgress()
                npcHandler:say(progress, npc, creature)
            else
                npcHandler:say("Congratulations! You have completed the mission!", npc, creature)
            end
            return true

        else
            npcHandler:say("You already have another active mission. Complete it first.", npc, creature)
            return true
        end
    end

    if message:find("start") then
        local activeMissionId = player:getActiveMissionId()
        if activeMissionId ~= 0 and activeMissionId ~= MISSION_ID then
            npcHandler:say("You already have another active mission. Complete it first.", npc, creature)
            return true
        end

        if not player:canStartStage(MISSION_ID) then
            local requirements = player:getStageRequirementsStatus(MISSION_ID)
            npcHandler:say("You do not meet the requirements to start this mission.\n\n" .. requirements, npc, creature)
            return true
        end

        player:setActiveMissionId(MISSION_ID)
        player:setActiveMissionStage(0)
        npcHandler:say("Your mission has begun! Talk to me anytime to see your progress.", npc, creature)
        return true
    end

    if message:find("next") then
        if player:getActiveMissionId() ~= MISSION_ID then
            npcHandler:say("You don't have the required mission active.", npc, creature)
            return true
        end

        if not player:checkStageCompletion() then
            npcHandler:say("You have not completed all objectives for this stage yet.", npc, creature)
            return true
        end

        local progress = player:getStageProgress()
        npcHandler:say("Stage advanced!\n\n" .. progress, npc, creature)
        return true
    end

    if message:find("deadpool") then
        player:completeTalkObjective("Mission Master")
        return true
    end

    return true
end

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:setMessage(MESSAGE_GREET, "Greetings, shinobi. Talk about {mission} if you seek to prove yourself.")
npcHandler:addModule(FocusModule:new(), npcConfig.name, true, true, true)

npcType:register(npcConfig)
