-- Exemplo de respawns por cidade (por townId)
local CITY_RESPAWNS = {
    [4] = { -- Konoha
		{x = 1228, y = 1002, z = 7},
		{x = 1225, y = 1002, z = 7},
		{x = 1231, y = 1002, z = 7},
		{x = 1229, y = 1003, z = 6},
		{x = 1238, y = 1004, z = 6},
		{x = 1238, y = 1006, z = 6},
		{x = 1226, y = 1020, z = 6},
		{x = 1235, y = 1004, z = 6},
		{x = 1229, y = 1004, z = 5},
		{x = 1227, y = 1004, z = 5},
		{x = 1225, y = 1004, z = 5},
		{x = 1236, y = 1004, z = 5},
		{x = 1238, y = 1004, z = 5},
		{x = 1238, y = 1006, z = 5},
		{x = 1226, y = 1020, z = 5},
		{x = 1236, y = 1006, z = 5},
        {x = 1236, y = 1002, z = 6},
        {x = 1228, y = 1003, z = 6},
        {x = 1238, y = 1007, z = 6},
        {x = 1237, y = 1008, z = 6},
        {x = 1225, y = 1005, z = 5},
        {x = 1237, y = 1006, z = 5},
        {x = 1229, y = 1005, z = 5},
        {x = 1227, y = 1005, z = 5}
    },
    [2] = { -- Suna
        {x = 637, y = 1105, z = 6},
        {x = 637, y = 1104, z = 6},
        {x = 638, y = 1107, z = 6},
        {x = 637, y = 1107, z = 6},
        {x = 639, y = 1105, z = 6},
        {x = 638, y = 1105, z = 6},
        {x = 633, y = 1104, z = 6},
        {x = 633, y = 1105, z = 6},
        {x = 633, y = 1107, z = 6},
        {x = 634, y = 1107, z = 6},
        {x = 635, y = 1105, z = 6},
        {x = 634, y = 1104, z = 6},
        {x = 629, y = 1104, z = 6},
        {x = 629, y = 1105, z = 6},
        {x = 629, y = 1107, z = 6},
        {x = 630, y = 1107, z = 6},
        {x = 631, y = 1105, z = 6},
        {x = 630, y = 1105, z = 6},
        {x = 637, y = 1104, z = 7},
        {x = 637, y = 1105, z = 7},
        {x = 637, y = 1107, z = 7},
        {x = 638, y = 1107, z = 7},
        {x = 639, y = 1105, z = 7},
        {x = 638, y = 1104, z = 7},
        {x = 633, y = 1104, z = 7}, 
        {x = 633, y = 1105, z = 7},
        {x = 633, y = 1107, z = 7},
        {x = 634, y = 1107, z = 7},
        {x = 635, y = 1105, z = 7},
        {x = 634, y = 1104, z = 7},
        {x = 629, y = 1104, z = 7},
        {x = 629, y = 1105, z = 7},
        {x = 629, y = 1107, z = 7},
        {x = 630, y = 1107, z = 7},
        {x = 631, y = 1105, z = 7},
        {x = 630, y = 1104, z = 7}
    },
    -- Adicione outros towns aqui!
}

local deathDescriptions = {
    "Here lies %s, who fell bravely in battle.",
    "RIP %s. Gone but not forgotten.",
    "%s met their end here.",
    "%s’s ninja journey ends at this spot.",
    "%s perished with honor.",
    "Here lies %s, who protected their village to the very end.",
    "%s’s shinobi path ends here, but their legacy lives on.",
    "%s died with their ninja way unbroken.",
    "%s’s sacrifice will never be forgotten by the Leaf.",
    "%s fell as a true hero of Konoha.",
    "%s stopped breathing, but never stopped fighting.",
    "\"Those who abandon their comrades are worse than scum.\" – %s died living this truth.",
    "Here rests %s, a ninja who believed in peace.",
    "%s fought until their last chakra.",
    "Even in death, %s’s spirit guards the village.",
    "\"Pain leads to maturity\" – %s knew both.",
    "The moon mourns %s, the sun honors them at dawn.",
    "\"I never go back on my word. That's my ninja way!\" – %s lived by it to the end.",
    "%s’s dreams now live in the hearts of others.",
    "The wind whispers %s’s name through the trees of the village.",
    "%s fell, but never failed.",
    "%s’s shadow still watches over their comrades.",
    "\"Loneliness is the fate of a ninja.\" – %s faced it with courage.",
    "%s died with their eyes set on a better world.",
    "\"To love is to give someone the power to destroy you.\" – %s loved too deeply.",
    "%s’s chakra faded, but their presence lingers.",
    "%s left behind a legacy greater than any jutsu.",
    "\"Those who break the rules are scum... but those who abandon their friends...\" – %s never did.",
    "%s died like a true shinobi: without regret.",
    "The tale of %s will echo through future generations.",
    "%s walked through darkness in search of light.",
    "Here lies %s, whose eyes saw the truth too late.",
    "\"Peace is never easy, but always worth it.\" – %s died believing that.",
    "%s was defeated, but never broken.",
    "May %s’s soul rest among the greatest ninjas in history."
}


local CEMETERY_ITEMS = {34515, 34514, 34513, 34512}

local CEMETERY_POSITIONS = {
    {x = 1278, y = 1019, z = 7}, {x = 1280, y = 1019, z = 7}, {x = 1282, y = 1019, z = 7},
    {x = 1287, y = 1019, z = 7}, {x = 1289, y = 1019, z = 7}, {x = 1291, y = 1019, z = 7},
    {x = 1278, y = 1021, z = 7}, {x = 1280, y = 1021, z = 7}, {x = 1282, y = 1021, z = 7},
    {x = 1287, y = 1021, z = 7}, {x = 1289, y = 1021, z = 7}, {x = 1291, y = 1021, z = 7},
    {x = 1278, y = 1023, z = 7}, {x = 1280, y = 1023, z = 7}, {x = 1282, y = 1023, z = 7},
    {x = 1287, y = 1023, z = 7}, {x = 1289, y = 1023, z = 7}, {x = 1291, y = 1023, z = 7},
    {x = 1278, y = 1025, z = 7}, {x = 1280, y = 1025, z = 7}, {x = 1282, y = 1025, z = 7},
    {x = 1287, y = 1025, z = 7}, {x = 1289, y = 1025, z = 7}, {x = 1291, y = 1025, z = 7},
    {x = 1278, y = 1027, z = 7}, {x = 1280, y = 1027, z = 7}, {x = 1282, y = 1027, z = 7},
    {x = 1287, y = 1027, z = 7}, {x = 1289, y = 1027, z = 7}, {x = 1291, y = 1027, z = 7}
}

local function findFreeCemeteryPosition()
    for _, pos in ipairs(CEMETERY_POSITIONS) do
        local tile = Tile(Position(pos.x, pos.y, pos.z))
        if tile and not tile:getTopDownItem() then
            return pos
        end
    end
    return nil
end

local function removeRandomTomb()
    for _, pos in ipairs(CEMETERY_POSITIONS) do
        local tile = Tile(Position(pos.x, pos.y, pos.z))
        if tile then
            local topItem = tile:getTopDownItem()
            if topItem and table.find(CEMETERY_ITEMS, topItem:getId()) then
                topItem:remove()
                return pos
            end
        end
    end
    return nil
end

local function hasPlayerTomb(playerName)
    for _, pos in ipairs(CEMETERY_POSITIONS) do
        local tile = Tile(Position(pos.x, pos.y, pos.z))
        if tile then
            local topItem = tile:getTopDownItem()
            if topItem and table.find(CEMETERY_ITEMS, topItem:getId()) then
                local desc = topItem:getAttribute(ITEM_ATTRIBUTE_DESCRIPTION)
                if desc and desc:find(playerName, 1, true) then
                    return true
                end
            end
        end
    end
    return false
end

local hospitalRespawnOnDeath = CreatureEvent("hospitalRespawnOnDeath")
function hospitalRespawnOnDeath.onDeath(player, corpse, killer, mostDamage, unjustified, mostDamage_unjustified)
    local town = player:getTown()
    local townId = town and town:getId()
    local respawns = townId and CITY_RESPAWNS[townId]
    if respawns and #respawns > 0 then
        local pos = respawns[math.random(#respawns)]
        player:kv():set("customRespawn", pos)
    else
        player:kv():set("customRespawn", {x=1000, y=1000, z=7})
    end

    if hasPlayerTomb(player:getName()) then
        return true
    end

    local freePos = findFreeCemeteryPosition() or removeRandomTomb()
    if not freePos then
        print("[CEMETERY] Falha ao encontrar posição para lápide.")
        return true
    end

    local tombPos = Position(freePos.x, freePos.y, freePos.z)
    local tombItemId = CEMETERY_ITEMS[math.random(#CEMETERY_ITEMS)]

    safePlayerEvent(player:getId(), 3000, function(uid)
        local player = Player(uid)
        if not player then return end
        local item = Game.createItem(tombItemId, 1, tombPos)
        if item then
            local desc = string.format(deathDescriptions[math.random(#deathDescriptions)], player:getName())
            item:setAttribute(ITEM_ATTRIBUTE_DESCRIPTION, desc)
        end
    end)

    return true
end
hospitalRespawnOnDeath:register()


local hospitalRespawnOnLogin = CreatureEvent("hospitalRespawnOnLogin")
function hospitalRespawnOnLogin.onLogin(player)
    local pos = player:kv():get("customRespawn")
    if pos then
        player:teleportTo(Position(pos.x, pos.y, pos.z))
        player:kv():remove("customRespawn")
    end

    player:registerEvent("hospitalRespawnOnDeath")
    return true
end
hospitalRespawnOnLogin:register()
