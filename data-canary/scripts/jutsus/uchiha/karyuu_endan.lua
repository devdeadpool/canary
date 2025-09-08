local spell = Spell("instant")

local hitRepeats = 2
local hitDelay = 10

local fireEffects = {
    [NORTH] = {effect = 590, dx = 3, dy = -1},
    [EAST]  = {effect = 593, dx = 8, dy = 0},
    [SOUTH] = {effect = 591, dx = 3, dy = 8},
    [WEST]  = {effect = 592, dx = -1, dy = 0},
}

-- MASK DA ÁREA (1 = atinge, 3 = centro/jogador)
local housenka_area = {
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 0 },
    { 0, 0, 1, 0, 0 },
    { 0, 0, 3, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
}

-- Gera as posições atingidas com base na máscara e direção do player
local function getMaskPositions(player, mask)
    -- Procura o centro da máscara
    local centerY, centerX
    for y = 1, #mask do
        for x = 1, #mask[y] do
            if mask[y][x] == 3 then
                centerY, centerX = y, x
                break
            end
        end
        if centerY then break end
    end

    local origin = player:getPosition()
    local dir = player:getDirection()
    local positions = {}

    for y = 1, #mask do
		for x = 1, #mask[y] do
			if mask[y][x] == 1 then
				local relY = y - centerY
				local relX = x - centerX
				local pos

				if dir == NORTH then
					pos = Position(origin.x + relX, origin.y + relY, origin.z)
				elseif dir == SOUTH then
					pos = Position(origin.x - relX, origin.y - relY, origin.z)
				elseif dir ==  WEST then
					pos = Position(origin.x + relY, origin.y + relX, origin.z) 
				elseif dir == EAST then
					pos = Position(origin.x - relY, origin.y - relX, origin.z)  
				end

				table.insert(positions, pos)
			end
		end
	end

    return positions
end

-- Só para efeito visual extra (na frente do player, como antes)
local function getFrontEffectPosition(player)
    local dir = player:getDirection()
    local pos = player:getPosition()
    local effect = fireEffects[dir]
    return Position(pos.x + effect.dx, pos.y + effect.dy, pos.z)
end

local function hitMaskWithFire(player, positions, hitCount, interval, combat)
    for hit = 1, hitCount do
        for i, pos in ipairs(positions) do
            safePlayerEvent(player:getId(), ((hit - 1) * #positions + i) * interval, function(p)
                pos:sendMagicEffect(3) -- DEBUG: pode mudar para efeito de fogo real ou tirar depois!
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
                    local maskPositions = getMaskPositions(p, housenka_area)
                    local dir = p:getDirection()
                    local frontEffectPos = getFrontEffectPosition(p)
                    frontEffectPos:sendMagicEffect(fireEffects[dir].effect)

                    safePlayerEvent(uid, 0, function(player)
                        hitMaskWithFire(player, maskPositions, hitRepeats, hitDelay, combat)
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
spell:register()
