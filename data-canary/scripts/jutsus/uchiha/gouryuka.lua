local spell = Spell("instant")

local function launchCurvedFireball(player, targetPos, delay, combat)
	local startPos = player:getPosition()
	local peakX = math.floor((startPos.x + targetPos.x) / 2)
	local peakY = math.floor((startPos.y + targetPos.y) / 2) - 3
	local peakPos = Position(peakX, peakY, startPos.z)
	local uid = player:getId()

	safePlayerEvent(uid, delay, function(p)
		startPos:sendDistanceEffect(peakPos, 72)
	end)

	safePlayerEvent(uid, delay + 200, function(p)
		peakPos:sendDistanceEffect(targetPos, 72)
	end)

	safePlayerEvent(uid, delay + 300, function(p)
		targetPos:sendMagicEffect(587)
		combat:execute(p, Variant(targetPos))
	end)
end

function spell.onCastSpell(player, variant)
	local level, ninjutsu = player:getNinjutsuLevel(), player:getLevel()
	local min = (level / 5) + (ninjutsu * 1.6) + 20
	local max = (level / 5) + (ninjutsu * 2.2) + 35

	local combat = Combat()
	combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
	combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
	combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -min, 0, -max)

	local uid = player:getId()

	local animate = createSpellAnimation({
		frames = {
			{ delay = 200, say = "Katon..." },
			{ delay = 500, outfit = function(p) return getUchihaJutsuOutfit("prepare", p) end, duration = 400 },
			{ delay = 900, say = "Gouryuka no Jutsu!", outfit = function(p) return getUchihaJutsuOutfit("cast", p) end, duration = 300 },
			{
				delay = 1200,
				cast = function()
					local player = Player(uid)
					if not player or player:isRemoved() then return end

					local target = player:getTarget()
					local basePos = (target and target:isCreature()) and target:getPosition() or player:getPosition()

					local offsets = {
						{0, 0}, {1, 0}, {-1, 0}, {0, -1}, {0, 1},
					}

					local totalRepeats = 3
					local delayBetween = 300

					for i = 1, totalRepeats do
						for j, offset in ipairs(offsets) do
							local dx, dy = offset[1], offset[2]
							local pos = Position(basePos.x + dx, basePos.y + dy, basePos.z)
							local totalDelay = (i - 1) * delayBetween + j * 60

							safePlayerEvent(uid, totalDelay, function(pl)
								launchCurvedFireball(pl, pos, 0, combat)
							end)
						end
					end
				end
			}
		}
	})

	animate(player, variant)
	return true
end

spell:name("Katon: Gouryuka no Jutsu")
spell:words("katon gouryuka no jutsu")
spell:group("attack")
spell:id(1006)
spell:needDirection(true)
--[[ spell:level(20)
spell:mana(90)
spell:magicLevel(9)
spell:needLearn(true)
spell:vocation("uchihaclan")
spell:cooldown(14 * 1000)
spell:groupCooldown(2000) ]]
spell:register()
