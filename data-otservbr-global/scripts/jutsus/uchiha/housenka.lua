local spell = Spell("instant")

local housenka_area = {
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0 },
	{ 0, 0, 3, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
}

local function getAreaPositions(player, area)
	local dir = player:getDirection()
	local origin = player:getPosition()
	local positions = {}

	local centerY = math.floor(#area / 2) + 1
	local centerX = math.floor(#area[1] / 2) + 1

	for y = 1, #area do
		for x = 1, #area[y] do
			if area[y][x] == 1 then
				local relX = x - centerX
				local relY = y - centerY
				local finalX, finalY

				if dir == NORTH then
					finalX = relX
					finalY = relY
				elseif dir == SOUTH then
					finalX = -relX
					finalY = -relY
				elseif dir == EAST then
					finalX = -relY
					finalY = relX
				elseif dir == WEST then
					finalX = relY
					finalY = -relX
				end

				local pos = Position(origin.x + finalX, origin.y + finalY, origin.z)
				table.insert(positions, pos)
			end
		end
	end

	return positions
end

function spell.onCastSpell(player, variant)
	local level, ninjutsu = player:getLevel(), player:getNinjutsuLevel()
	local min = (level / 5) + (ninjutsu * 1.1) + 12
	local max = (level / 5) + (ninjutsu * 1.6) + 20

	local combat = Combat()
	combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
	combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
	combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -min, 0, -max)

	local uid = player:getId()

	local animate = createSpellAnimation({
		frames = {
			{ delay = 500, outfit = function(p) return getUchihaJutsuOutfit("prepare", p) end, duration = 100 },
			{ delay = 500, say = "Housenka no Jutsu!", outfit = function(p) return getUchihaJutsuOutfit("cast", p) end, duration = 100 },
			{
				delay = 100,
				cast = function()
					local player = Player(uid)
					if not player or player:isRemoved() then return end

					local area = getAreaPositions(player, housenka_area)
					for i = #area, 2, -1 do
						local j = math.random(i)
						area[i], area[j] = area[j], area[i]
					end

					for i, pos in ipairs(area) do
						local delay = i * 40

						safePlayerEvent(uid, delay, function(p)
							p:getPosition():sendDistanceEffect(pos, CONST_ANI_FIRE)
						end)

						safePlayerEvent(uid, delay + 300, function(p)
							pos:sendMagicEffect(CONST_ME_FIREAREA)
							combat:execute(p, Variant(pos))
						end)
					end
				end
			}
		}
	})

	animate(player, variant)
	return true
end

spell:name("Katon: Housenka no Jutsu")
spell:words("katon housenka no jutsu")
spell:group("attack")
spell:id(1003)
spell:needDirection(true)
--[[ spell:blockWalls(true)
spell:isAggressive(true)
spell:level(12)
spell:mana(60)
spell:magicLevel(6)
spell:needLearn(true)
spell:vocation("uchihaclan")
spell:cooldown(10 * 1000)
spell:groupCooldown(2000) ]]
spell:register()
