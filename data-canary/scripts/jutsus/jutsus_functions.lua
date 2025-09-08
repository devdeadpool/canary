function createSpellAnimation(config)
	return function(player, variant)
		local uid = player:getId()
		local delay = 0

		for _, frame in ipairs(config.frames) do
			delay = delay + (frame.delay or 0)

			addEvent(function()
				local caster = Player(uid)
				if not caster then return end

				if frame.say then
					caster:say(frame.say, TALKTYPE_MONSTER_SAY)
				end

				if frame.outfit then
					local outfitData
					if type(frame.outfit) == "function" then
						outfitData = frame.outfit(caster)
					else
						outfitData = frame.outfit
					end

					if outfitData then
						local current = caster:getOutfit()
						local condition = Condition(CONDITION_OUTFIT)
						condition:setOutfit({
							lookType = outfitData.lookType,
							lookHead = current.lookHead,
							lookBody = current.lookBody,
							lookLegs = current.lookLegs,
							lookFeet = current.lookFeet,
							lookAddons = current.lookAddons,
							lookMount = current.lookMount
						})
						condition:setTicks(frame.duration or 500)
						caster:addCondition(condition)
					end
				end

				if frame.effect then
					caster:getPosition():sendMagicEffect(frame.effect)
				end

				if type(frame.cast) == "function" then
					frame.cast(caster, variant)
				end
			end, delay)
		end
	end
end

function getLineBetween(pos1, pos2)
	local path = {}
	local x1, y1 = pos1.x, pos1.y
	local x2, y2 = pos2.x, pos2.y
	local dx = math.abs(x2 - x1)
	local dy = math.abs(y2 - y1)
	local sx = x1 < x2 and 1 or -1
	local sy = y1 < y2 and 1 or -1
	local err = dx - dy

	while true do
		table.insert(path, Position(x1, y1, pos1.z))
		if x1 == x2 and y1 == y2 then break end
		local e2 = 2 * err
		if e2 > -dy then err = err - dy; x1 = x1 + sx end
		if e2 < dx  then err = err + dx; y1 = y1 + sy end
	end

	return path
end

function safePlayerEvent(uid, delay, callback)
	addEvent(function()
		local player = Player(uid)
		if not player or player:isRemoved() then return end
		callback(player)
	end, delay)
end
