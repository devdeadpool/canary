local spell = Spell("instant")

-- Setup de combate base
local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)

local explosionEffect = 472
local explosionOffsets = {
    [NORTH] = {dx = 4, dy = 4},
    [SOUTH] = {dx = 4, dy = 3},
    [EAST]  = {dx = 4, dy = 4},
    [WEST]  = {dx = 2, dy = 4},
}

-- Condição de queimadura por 30s
local burnCondition = Condition(CONDITION_FIRE)
burnCondition:setParameter(CONDITION_PARAM_TICKS, 30000)
burnCondition:setParameter(CONDITION_PARAM_DAMAGEPERSECOND, 25)

-- Função para obter posições de linha na direção do player
local function getLinePositions(player, length)
    local dir = player:getDirection()
    local pos = player:getPosition()
    local result = {}
    for i = 1, length do
        local tile = Position(pos)
        if dir == DIRECTION_NORTH then tile.y = tile.y - i
        elseif dir == DIRECTION_SOUTH then tile.y = tile.y + i
        elseif dir == DIRECTION_EAST then tile.x = tile.x + i
        elseif dir == DIRECTION_WEST then tile.x = tile.x - i end
        table.insert(result, tile)
    end
    return result
end

-- Função para gerar área 3x3 centrada numa posição
local function getArea3x3(centerPos)
    local area = {}
    for dx = -1, 1 do
        for dy = -1, 1 do
            table.insert(area, Position(centerPos.x + dx, centerPos.y + dy, centerPos.z))
        end
    end
    return area
end

function spell.onCastSpell(player, variant)
    local uid = player:getId()
    local level, ninjutsu = player:getLevel(), player:getNinjutsuLevel()
    local min = (level / 5) + (ninjutsu * 2.0) + 25
    local max = (level / 5) + (ninjutsu * 2.8) + 40
    combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -min, 0, -max)

    local line = getLinePositions(player, 7)
    local creaturesToPull = {}

    -- Detecta TODAS as criaturas puxáveis na linha
    for i, pos in ipairs(line) do
        local tile = Tile(pos)
        if tile then
            local creature = tile:getTopCreature()
            if creature and creature ~= player then
                if creature:isPlayer() or (creature:isMonster() and not creature:isNpc()) then
                    table.insert(creaturesToPull, creature)
                end
            end
        end
    end

    local animate = createSpellAnimation({
        frames = {
            { delay = 300, say = "Katon..." },
            { delay = 600, outfit = function(p) return getUchihaJutsuOutfit("prepare", p) end, duration = 400 },
            { delay = 1000, say = "Ryūka no Jutsu!", outfit = function(p) return getUchihaJutsuOutfit("cast", p) end, duration = 300 },
            {
                delay = 1300,
                cast = function(p)
                    -- Linha de fogo
                    for i, pos in ipairs(line) do
                        safePlayerEvent(uid, i * 50, function(plr)
                            pos:sendMagicEffect(589)
                        end)
                    end

                    -- Distance effect do jogador até o final
                    local finalPos = line[#line]
                    safePlayerEvent(uid, 100, function(plr)
                        Position(plr:getPosition()):sendDistanceEffect(finalPos, 45)
                    end)

                    -- Puxar todas criaturas para o final
                    safePlayerEvent(uid, #line * 50, function(plr2)
                        for _, creature in ipairs(creaturesToPull) do
                            if creature and creature:isCreature() and creature ~= plr2 then
                                creature:teleportTo(finalPos, true)
                               -- finalPos:sendMagicEffect(CONST_ME_ENERGYHIT)
                            end
                        end
                    end)

                    -- Explosão final: aplica dano/burn em todo 3x3
                   safePlayerEvent(uid, (#line) * 50 + 100, function(plr2)
                      local finalPos = line[#line]
                      local dir = plr2:getDirection()
                      local off = explosionOffsets[dir] or {dx = 0, dy = 0}
                      local effectPos = Position(finalPos.x + off.dx, finalPos.y + off.dy, finalPos.z)
                      effectPos:sendMagicEffect(explosionEffect)
                      -- Dano/efeito area 3x3 (igual antes)
                      local area = getArea3x3(finalPos)
                      for _, pos in ipairs(area) do
                          combat:execute(plr2, Variant(pos))
                          local tile = Tile(pos)
                          if tile then
                              local target = tile:getTopCreature()
                              if target and target ~= plr2 then
                                  target:addCondition(burnCondition)
                                  target:getPosition():sendMagicEffect(CONST_ME_HITBYFIRE)
                              end
                          end
                      end
                  end)

                end
            }
        }
    })

    animate(player, variant)
    return true
end

spell:name("Katon: Ryuuka no Jutsu")
spell:words("katon ryuuka")
spell:group("attack")
spell:id(1011)
spell:needDirection(true)
--[[ spell:level(20)
spell:mana(80)
spell:magicLevel(9)
spell:needLearn(true)
spell:vocation("uchihaclan")
spell:cooldown(14 * 1000)
spell:groupCooldown(2000) ]]
spell:register()
