local spell = Spell("instant")

local combat = Combat()

local area = createCombatArea(AREA_CIRCLE3X4)
combat:setArea(area)
combat:setParameter(COMBAT_PARAM_EFFECT, 16)
combat:setParameter(COMBAT_PARAM_DISPEL, CONDITION_INVISIBLE)
combat:setParameter(COMBAT_PARAM_AGGRESSIVE, false)

local condition = Condition(CONDITION_PARALYZE)
condition:setParameter(CONDITION_PARAM_TICKS, 5 * 1000)
condition:setFormula(-1, 0, -1, 0)
combat:addCondition(condition)

local damageDebuff = Condition(CONDITION_ATTRIBUTES)
damageDebuff:setParameter(CONDITION_PARAM_TICKS, 5 * 1000)
damageDebuff:setParameter(CONDITION_PARAM_BUFF_DAMAGEDEALT, 50)
combat:addCondition(damageDebuff)




-- Aplica os efeitos em cada criatura na tile


function spell.onCastSpell(player, variant)
    if not player:isPetSummoned() then
        player:sendCancelMessage("Você precisa ter o Akamaru invocado para usar este jutsu.")
        return false
    end 
    print('asd')
    local uid = player:getId()

    local animate = createSpellAnimation({
        frames = {
            { delay = 200, say = "Akamaru, dinamikku mākingu!" },
            {
                delay = 500,
                cast = function(p)
                    local caster = Player(uid)
                    if not caster or caster:isRemoved() then return end
                    end
            }
        }
    })

    animate(player, variant)
    return combat:execute(player, variant)
   --[[  return true ]]
end

spell:name("Inuzuka: Dynamic Marking")
spell:words("dynamic marking")
spell:group("attack")
spell:id(1005)
spell:needDirection(true)
--[[ spell:level(12)
spell:mana(50)
spell:cooldown(10000)
spell:groupCooldown(2000)
spell:vocation("inuzukaclan")
spell:needLearn(true) ]]
spell:register()
