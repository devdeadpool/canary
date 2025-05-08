local spell = Spell("instant")

local fireLength = 6
local fireWidth = 3
local hitRepeats = 3
local hitDelay = 100

local fireEffects = {
	[NORTH] = {effect = 184, dx = 0, dy = -1},
	[EAST]  = {effect = 182, dx = 1, dy = 0},
	[SOUTH] = {effect = 183, dx = 0, dy = 1},
	[WEST]  = {effect = 179, dx = -1, dy = 0},
}

local function getLineAreaPositions(player, length, width)
	local dir = player:getDirection()
	local origin = player:getPosition()
	local offset = fireEffects[dir]
	local positions = {}

	local halfWidth = math.floor(width / 2)

	for i = 1, length do
		for w = -halfWidth, halfWidth do
			local pos
			if dir == NORTH or dir == SOUTH then
				pos = Position(origin.x + w, origin.y + offset.dy * i, origin.z)
			else
				pos = Position(origin.x + offset.dx * i, origin.y + w, origin.z)
			end
			table.insert(positions, pos)
		end
	end
	return positions
end

local function hitLineWithFire(player, positions, hitCount, interval, combat)
	for hit = 1, hitCount do
		for i, pos in ipairs(positions) do
			safePlayerEvent(player:getId(), ((hit - 1) * #positions + i) * interval, function(p)
				pos:sendMagicEffect(fireEffects[p:getDirection()].effect)
				combat:execute(p, Variant(pos))
			end)
		end
	end
end

function spell.onCastSpell(player, variant)
	local level = player:getLevel()
	local ninjutsu = player:getNinjutsuLevel()
	local min = (level / 5) + (ninjutsu * 1.6) + 15
	local max = (level / 5) + (ninjutsu * 2.3) + 25

	local combat = Combat()
	combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
	combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
	combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -min, 0, -max)

	local animate = createSpellAnimation({
		frames = {
			{ delay = 200, say = "Katon..." },
			{ delay = 500, outfit = function(p) return getUchihaJutsuOutfit("prepare", p) end, duration = 400 },
			{ delay = 900, say = "Karyuu Endan!", outfit = function(p) return getUchihaJutsuOutfit("cast", p) end, duration = 300 },
			{
				delay = 1200,
				cast = function(p)
					local uid = p:getId()
					local line = getLineAreaPositions(p, fireLength, fireWidth)
					safePlayerEvent(uid, 0, function(player)
						hitLineWithFire(player, line, hitRepeats, hitDelay, combat)
					end)
				end
			}
		}
	})

	animate(player, variant)
	return true
end

spell:name("Katon: Karyuu Endan")
spell:words("katon karyuu endan")
spell:group("attack")
spell:id(1008)
spell:needDirection(true)
--[[ spell:level(22)
spell:mana(100)
spell:magicLevel(10)
spell:needLearn(true)
spell:vocation("uchihaclan")
spell:cooldown(16 * 1000)
spell:groupCooldown(2000) ]]
spell:register()
