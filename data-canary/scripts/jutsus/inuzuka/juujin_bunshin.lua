local spell = Spell("instant")

function spell.onCastSpell(player, variant)
    if not player:isPetSummoned() then
        player:sendCancelMessage("Você precisa ter o Akamaru invocado para usar este jutsu.")
        return false
    end

    local pet = nil
    for _, summon in ipairs(player:getSummons()) do
        if summon:getName():lower():find("akamaru") then
            pet = summon
            break
        end
    end

    if not pet or pet:isRemoved() then
        player:sendCancelMessage("Akamaru precisa estar presente para usar este jutsu.")
        return false
    end

    local duration = 10 -- segundos
    local outfit = player:getOutfit()

    -- Condition visual (outfit do player)
    local conditionOutfit = Condition(CONDITION_OUTFIT)
    conditionOutfit:setTicks(duration * 1000)
    conditionOutfit:setOutfit(outfit)

    -- Condition de atributos (buff Taijutsu + speed)
    local conditionAttr = Condition(CONDITION_ATTRIBUTES)
    conditionAttr:setTicks(duration * 1000)
    conditionAttr:setParameter(CONDITION_PARAM_STAT_MELEE, 15) -- boost temporário no dano corpo a corpo
    conditionAttr:setParameter(CONDITION_PARAM_SPEED, 120) -- boost leve de velocidade

    -- Aplicar ao pet
    pet:addCondition(conditionOutfit)
    pet:addCondition(conditionAttr)

    -- Flag no KV para futuras verificações
    player:kv():set("juujin_active", true)

    -- Remover a flag depois do tempo
    safePlayerEvent(player:getId(), duration * 1000, function(uid)
        local p = Player(uid)
        if p then
            p:kv():remove("juujin_active")
        end
    end)

    -- Efeito e fala
    player:say("Juujin Bunshin!", TALKTYPE_MONSTER_SAY)
    pet:getPosition():sendMagicEffect(CONST_ME_MAGIC_RED)

    return true
end

spell:name("Inuzuka: Juujin Bunshin")
spell:words("juujin bunshin")
spell:group("support")
spell:id(1007)
--[[ spell:level(10)
spell:mana(50)
spell:cooldown(10000)
spell:groupCooldown(2000)
spell:vocation("inuzukaclan")
spell:needLearn(true) ]]
spell:register()
