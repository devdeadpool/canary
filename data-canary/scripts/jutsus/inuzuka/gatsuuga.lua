local spell = Spell("instant")

function spell.onCastSpell(player, variant)
    local hasPet = player:isPetSummoned()
    local pet = nil

    if hasPet then
        for _, summon in ipairs(player:getSummons()) do
            if summon:getName():lower():find("akamaru") then
                pet = summon
                break
            end
        end
    end

    local uid = player:getId()
    local pid = pet and pet:getId()

    local function getAdvancePositions(pos, dir, distance)
        local offset = {
            [DIRECTION_NORTH] = {x = 0, y = -1},
            [DIRECTION_EAST]  = {x = 1, y = 0},
            [DIRECTION_SOUTH] = {x = 0, y = 1},
            [DIRECTION_WEST]  = {x = -1, y = 0}
        }
        local dx, dy = offset[dir].x, offset[dir].y
        local positions = {}
        for i = 1, distance do
            table.insert(positions, Position(pos.x + dx * i, pos.y + dy * i, pos.z))
        end
        return positions
    end

    local function doGatsuugaAdvance(creature)
        local isPlayer = creature:isPlayer()
        local cid = creature:getId()
        local dir = creature:getDirection()
        local startPos = creature:getPosition()
        local path = getAdvancePositions(startPos, dir, 3)
        local finalPos = path[#path]

        for _, pos in ipairs(path) do
            pos:sendMagicEffect(CONST_ME_DRAWBLOOD)
            local target = Tile(pos):getTopCreature()
            if target and target:isCreature() and target ~= creature then
                local marked = target:isPlayer() and target:kv():get("markedByDynamic").value_or(false) or false

                local level, taijutsu
                if isPlayer then
                    level = creature:getLevel()
                    taijutsu = creature:getSkillLevel(SKILL_TAIJUTSU)
                else
                    level = player:getPetLevel()
                    taijutsu = player:getPetAttack()
                end

                local baseMin = (level / 5) + (taijutsu * 1.5) + 20
                local baseMax = (level / 5) + (taijutsu * 2.1) + 35
                if marked then
                    baseMin = baseMin * 1.5
                    baseMax = baseMax * 1.5
                end

                local combat = Combat()
                combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
                combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
                combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -baseMin, 0, -baseMax)
                local source = creature:isPlayer() and creature or player
                combat:execute(source, Variant(pos))
            end
        end

        if Tile(finalPos):isWalkable() then
            creature:teleportTo(finalPos)
            finalPos:sendMagicEffect(CONST_ME_TELEPORT)
        else
            startPos:sendMagicEffect(CONST_ME_POFF)
        end
    end

    local animate = createSpellAnimation({
        frames = {
            { delay = 200, say = "Gatsuuga!!" },
            {
                delay = 500,
                cast = function()
                    local caster = Player(uid)
                    if not caster or caster:isRemoved() then return end

                    doGatsuugaAdvance(caster)
                    -- Se Juujin estiver ativo e pet válido, executar também
                    local juujinActive = false
                    if caster and caster:isPlayer() then
                        juujinActive = caster:kv():get("juujin_active") or false
                    end
                    print(juujinActive)

                    if juujinActive and pid then
                        local summon = Creature(pid)
                        if summon and not summon:isRemoved() then
                            doGatsuugaAdvance(summon)
                        end
                    end
                end
            }
        }
    })

    animate(player, variant)
    return true
end

spell:name("Inuzuka: Gatsuuga")
spell:words("gatsuuga")
spell:group("attack")
spell:id(1006)
spell:needDirection(true)
--[[ spell:level(14)
spell:mana(70)
spell:cooldown(8000)
spell:groupCooldown(2000)
spell:vocation("inuzukaclan")
spell:needLearn(true) ]]
spell:register()
