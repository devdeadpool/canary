local spell = Spell("instant")

-- Setup de combate base
local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
combat:setArea(createCombatArea({
  {1, 1, 1},
  {1, 3, 1},
  {1, 1, 1},
}))

-- Condição de queimadura por 30s
local burnCondition = Condition(CONDITION_FIRE)
burnCondition:setParameter(CONDITION_PARAM_TICKS, 30000)
burnCondition:setParameter(CONDITION_PARAM_DAMAGEPERSECOND, 25)

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

local function doKnockback(centerPos, player)
  local area = combat:getArea():getPositions(centerPos)
  for _, pos in ipairs(area) do
    local tile = Tile(pos)
    if tile then
      local creature = tile:getTopCreature()
      if creature and creature ~= player and creature:isMoveable() then
        local dx = pos.x - centerPos.x
        local dy = pos.y - centerPos.y
        local dest = Position(pos.x + dx, pos.y + dy, pos.z)
        if Tile(dest) and not Tile(dest):hasFlag(TILESTATE_BLOCKSOLID) then
          creature:teleportTo(dest, true)
        end
      end
    end
  end
end

function spell.onCastSpell(player, variant)
  local uid = player:getId()
  local level, ninjutsu = player:getLevel(), player:getNinjutsuLevel()
  local min = (level / 5) + (ninjutsu * 2.0) + 25
  local max = (level / 5) + (ninjutsu * 2.8) + 40
  combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -min, 0, -max)

  local line = getLinePositions(player, 7)
  local draggedCreature, dragIndex = nil, 0

  -- Detecta criatura empurrável
  for i, pos in ipairs(line) do
    local tile = Tile(pos)
    if tile then
      local creature = tile:getTopCreature()
      if creature and creature ~= player then
        if creature:isMonster() then
          local mtype = MonsterType(creature:getName())
          if mtype and mtype:isPushable() then
            draggedCreature = creature
            dragIndex = i
            break
          end
        elseif creature:isPlayer() then
          draggedCreature = creature
          dragIndex = i
          break
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
            safePlayerEvent(uid, i * 100, function(plr)
              pos:sendMagicEffect(CONST_ME_FIREATTACK)
            end)
          end

          -- Distance effect do jogador até o final
          local finalPos = line[#line]
          safePlayerEvent(uid, 100, function(plr)
            Position(plr:getPosition()):sendDistanceEffect(finalPos, CONST_ANI_FIRE)
          end)

          -- Empurrar criatura
          if draggedCreature then
            for i = dragIndex + 1, #line do
              local delay = (i - dragIndex) * 150
              local nextPos = line[i]
              safePlayerEvent(uid, delay, function()
                if draggedCreature then
                  draggedCreature:teleportTo(nextPos, true)
                  nextPos:sendMagicEffect(CONST_ME_FIREATTACK)
                end
              end)
            end

            -- Explosão final + dano + queimadura + knockback + efeito
            safePlayerEvent(uid, (#line - dragIndex) * 150 + 200, function()
              if draggedCreature then
                finalPos:sendMagicEffect(CONST_ME_FIREAREA)
                combat:execute(player, Variant(finalPos))
                draggedCreature:addCondition(burnCondition)
                draggedCreature:getPosition():sendMagicEffect(CONST_ME_HITBYFIRE)
                doKnockback(finalPos, player)
              end
            end)

          else
            -- Sem criatura
            safePlayerEvent(uid, #line * 100 + 200, function(plr2)
              finalPos:sendMagicEffect(CONST_ME_FIREAREA)
              combat:execute(plr2, Variant(finalPos))
              doKnockback(finalPos, player)

              -- Aplicar queimadura e efeito visual em monstros no 3x3
              local area = combat:getArea():getPositions(finalPos)
              for _, pos in ipairs(area) do
                local tile = Tile(pos)
                if tile then
                  local creature = tile:getTopCreature()
                  if creature and creature ~= player then
                    creature:addCondition(burnCondition)
                    creature:getPosition():sendMagicEffect(CONST_ME_HITBYFIRE)
                  end
                end
              end
            end)
          end
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
