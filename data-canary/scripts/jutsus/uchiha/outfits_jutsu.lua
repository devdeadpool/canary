UchihaJutsuOutfits = {
    [GRADUATIONS.ACADEMY] = {
        prepare = { lookType = 101 },
        cast    = { lookType = 102 },
        final     = { lookType = 103 },
    },
    [GRADUATIONS.GENIN] = {
        prepare = { lookType = 3 },
        cast    = { lookType = 4 },
    }
}

function getUchihaJutsuOutfit(stage, player)
    local rank = player:getGraduation() or "Academy Student"
    local rankTable = UchihaJutsuOutfits[rank]
    return rankTable and rankTable[stage] or nil
end