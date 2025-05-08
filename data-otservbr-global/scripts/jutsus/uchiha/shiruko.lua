local spell = Spell("instant")

local AREA_SHIRUKO = {
	{0, 0, 1, 0, 0},
	{0, 1, 1, 1, 0},
	{1, 1, 1, 1, 1},
	{0, 1, 1, 1, 0},
	{0, 0, 3, 0, 0},
	{0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0},
}

local function getLayeredPositions(player, area)
	local dir = player:getDirection()
	local origin = player:getPosition()
	local centerY = math.floor(#area / 2) + 1
	local centerX = math.floor(#area[1] / 2) + 1

	local layers = {}

	for y = 1, #area do
		for x = 1, #area[y] do
			if area[y][x] == 1 then
				local relX = x - centerX
				local relY = y - centerY
				local fx, fy

				if dir == NORTH then fx = relX; fy = relY
				elseif dir == SOUTH then fx = -relX; fy = -relY
				elseif dir == EAST  then fx = -relY; fy = relX
				elseif dir == WEST  then fx = relY; fy = -relX end

				local worldPos = Position(origin.x + fx, origin.y + fy, origin.z)

				local layerKey = (dir == NORTH or dir == SOUTH) and fy or fx
				layers[layerKey] = layers[layerKey] or {}
				table.insert(layers[layerKey], worldPos)
			end
		end
	end

	local sorted = {}
	local keys = {}
	for k in pairs(layers) do table.insert(keys, k) end
	table.sort(keys)

	for _, k in ipairs(keys) do
		table.insert(sorted, layers[k])
	end

	return sorted
end

function spell.onCastSpell(player, variant)
	local level, ninjutsu = player:getLevel(), player:getNinjutsuLevel()
	local min = (level / 5) + (ninjutsu * 1.3) + 14
	local max = (level / 5) + (ninjutsu * 1.8) + 25

	local combat = Combat()
	combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_FIREDAMAGE)
	combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
	combat:setFormula(COMBAT_FORMULA_LEVELMAGIC, 0, -min, 0, -max)

	local uid = player:getId()

	local animate = createSpellAnimation({
		frames = {
			{ delay = 250, say = "Katon..." },
			{ delay = 500, outfit = function(p) return getUchihaJutsuOutfit("prepare", p) end, duration = 400 },
			{ delay = 800, say = "Shiruko no Jutsu!", outfit = function(p) return getUchihaJutsuOutfit("cast", p) end, duration = 300 },
			{
				delay = 1100,
				cast = function()
					local p = Player(uid)
					if not p then return end

					local fx = {
						[NORTH] = {id = 184, x = 0, y = -1},
						[EAST]  = {id = 182, x = 2, y = 0},
						[SOUTH] = {id = 183, x = 0, y = 2},
						[WEST]  = {id = 179, x = -2, y = 0},
					}
					local d = fx[p:getDirection()]
					Position(p:getPosition().x + d.x, p:getPosition().y + d.y, p:getPosition().z):sendMagicEffect(d.id)

					local layers = getLayeredPositions(p, AREA_SHIRUKO)
					for index, positions in ipairs(layers) do
						safePlayerEvent(uid, index * 100, function(player)
							for _, pos in ipairs(positions) do
								pos:sendMagicEffect(CONST_ME_FIREAREA)
								combat:execute(player, Variant(pos))
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

spell:name("Katon: Shiruko no Jutsu")
spell:words("katon shiruko no jutsu")
spell:group("attack")
spell:id(1005)
spell:needDirection(true)
--[[ spell:level(16)
spell:mana(70)
spell:magicLevel(7)
spell:needLearn(true)
spell:vocation("uchihaclan")
spell:cooldown(12 * 1000)
spell:groupCooldown(2000) ]]
spell:register()
