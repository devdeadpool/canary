local spell = Spell("instant")

local AREA_HAISEKISHOU = {
  {1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1},
}

local function getCenteredArea(player, area)
  local centerY = math.floor(#area / 2) + 1
  local centerX = math.floor(#area[1] / 2) + 1
  local origin = player:getPosition()
  local result = {}

  for y = 1, #area do
    for x = 1, #area[y] do
      if area[y][x] == 1 then
        local relX = x - centerX
        local relY = y - centerY
        table.insert(result, Position(origin.x + relX, origin.y + relY, origin.z))
      end
    end
  end

  return result
end

function spell.onCastSpell(player, variant)
  local level, ninjutsu = player:getLevel(), player:getNinjutsuLevel()
  local min = (level / 5) + (ninjutsu * 1.4) + 14
  local max = (level / 5) + (ninjutsu * 2.0) + 22

  local combat = Combat()
  combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
  combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
  combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -min, 0, -max)

  local animate = createSpellAnimation({
    frames = {
      { delay = 300, say = "Katon..." },
      { delay = 600, outfit = function(p) return getUchihaJutsuOutfit("prepare", p) end, duration = 400 },
      { delay = 1000, say = "Haisekishou!", outfit = function(p) return getUchihaJutsuOutfit("cast", p) end, duration = 300 },
      {
        delay = 1000,
        cast = function(p)
          local uid = p:getId()
          local positions = getCenteredArea(p, AREA_HAISEKISHOU)
          local posX = 3
          local posY = 2
          local position = player:getPosition()
          local modifiedPos = Position(position.x + posX, position.y + posY, position.z)
          modifiedPos :sendMagicEffect(280)

          safePlayerEvent(uid, 1000, function(player)
            for _, pos in ipairs(positions) do
              combat:execute(player, Variant(pos))
            end
          end)
        end
      }
    }
  })

  animate(player, variant)
  return true
end

spell:name("Katon: Haisekishou")
spell:words("katon haisekishou")
spell:group("attack")
spell:id(1008)
spell:needDirection(false)
--[[ spell:level(18)
spell:mana(85)
spell:magicLevel(8)
spell:needLearn(true)
spell:vocation("uchihaclan")
spell:cooldown(14 * 1000)
spell:groupCooldown(2000) ]]
spell:register()
