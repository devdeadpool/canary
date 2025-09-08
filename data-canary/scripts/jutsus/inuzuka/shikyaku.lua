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

    if player:kv():get("shikyaku_active") then
        player:sendCancelMessage("Você já está sob o efeito do Shikyaku no Jutsu.")
        return false
    end

    local uid = player:getId()
    local pid = pet:getId()
    local duration = 10 -- segundos
    local speedBonus = 250
    local taijutsuBonus = 8
    local outfitId = 176 -- outfit estilo feral



    local meleeCondition = Condition(CONDITION_ATTRIBUTES)
    meleeCondition:setParameter(CONDITION_PARAM_SUBID, JeanPierreMelee)
    meleeCondition:setParameter(CONDITION_PARAM_BUFF_SPELL, 1)
    meleeCondition:setParameter(CONDITION_PARAM_TICKS, duration * 1000)
    meleeCondition:setParameter(CONDITION_PARAM_SKILL_MELEE, taijutsuBonus)
    meleeCondition:setParameter(CONDITION_PARAM_FORCEUPDATE, true)

    local speedCondition = Condition(CONDITION_HASTE)
    speedCondition:setParameter(CONDITION_PARAM_TICKS, duration * 1000)
    speedCondition:setParameter(CONDITION_PARAM_SPEED, speedBonus)
    speedCondition:setParameter(CONDITION_PARAM_FORCEUPDATE, true)

    local function applyShikyakuEffects()
        local p = Player(uid)
        local s = Creature(pid)

        if not p or p:isRemoved() then return end

        -- Falas
        p:say("Grrrrrr!", TALKTYPE_MONSTER_SAY)
        if s and not s:isRemoved() then
            s:say("Grrrrrr!", TALKTYPE_MONSTER_SAY)
        end

        local current = p:getOutfit()
        local condOutfit = Condition(CONDITION_OUTFIT)
        condOutfit:setParameter(CONDITION_PARAM_TICKS, duration * 1000)
        condOutfit:setOutfit({
            lookType = outfitId,
            lookHead = current.lookHead,
            lookBody = current.lookBody,
            lookLegs = current.lookLegs,
            lookFeet = current.lookFeet,
            lookAddons = current.lookAddons
        })

        p:addCondition(meleeCondition)
        p:addCondition(speedCondition)
        p:addCondition(condOutfit)

        -- Buff visual
        p:getPosition():sendMagicEffect(CONST_ME_ENERGYAREA)

        -- Pet outfit
        if s and not s:isRemoved() then
            local sOutfit = s:getOutfit()
            local sCond = Condition(CONDITION_OUTFIT)
            sCond:setParameter(CONDITION_PARAM_TICKS, duration * 1000)
            sCond:setOutfit({
                lookType = outfitId,
                lookHead = sOutfit.lookHead,
                lookBody = sOutfit.lookBody,
                lookLegs = sOutfit.lookLegs,
                lookFeet = sOutfit.lookFeet,
                lookAddons = sOutfit.lookAddons
            })
            s:addCondition(sCond)
        end

        -- Flag para outras magias
        p:kv():set("shikyaku_active", true)

        -- Desmarcar
        addEvent(function(cid)
            local playerCheck = Player(cid)
            if playerCheck and not playerCheck:isRemoved() then
                playerCheck:kv():remove("shikyaku_active")
            end
        end, duration * 1000, uid)
    end

    local animate = createSpellAnimation({
        frames = {
            { delay = 200, say = "Shikyaku no Jutsu!" },
            { delay = 400, cast = applyShikyakuEffects }
        }
    })

    animate(player, variant)
    return true
end

spell:name("Inuzuka: Shikyaku no Jutsu")
spell:words("shikyaku no jutsu")
spell:group("support")
spell:id(1007)
spell:needDirection(false)
--[[ spell:level(10)
spell:mana(60)
spell:cooldown(12000)
spell:groupCooldown(2000)
spell:vocation("inuzukaclan")
spell:needLearn(true) ]]
spell:register()
