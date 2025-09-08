-- data/scripts/systems/thermal_fountain.lua

local config = {
	kvKey = "insideThermalFountain",
	denyMsg = {
		[0] = "Access denied! This source is exclusive to women.",
		[1] = "Access denied! This source is exclusive to men.",
	},
	enterMsg = {
		[0] = "Welcome to the thermal fountain for women.",
		[1] = "Welcome to the thermal fountain for men.",
	},
	exitMsg = "You left the thermal fountain.",
}

local function getFountainGenderByAID(aid)
	if aid == GLOBAL_SYSTEMS.thermal.male then
		return 1
	elseif aid == GLOBAL_SYSTEMS.thermal.female then
		return 0
	end
	return nil
end

local thermalFountain = MoveEvent()

function thermalFountain.onStepIn(creature, item, position, fromPosition)
	local player = creature:getPlayer()
	if not player then return true end

	local gender = getFountainGenderByAID(item.actionid)
	if gender == nil then return true end

	local kv = player:kv()
	local wasInside = kv:get(config.kvKey) or false

	if wasInside then
		-- Saindo da fonte
		kv:set(config.kvKey, false)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, config.exitMsg)
	else
		-- Entrando na fonte
		if player:getSex() ~= gender then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, config.denyMsg[gender])
			position:sendMagicEffect(CONST_ME_POFF)
			player:teleportTo(fromPosition, false)
			return true
		end

		kv:set(config.kvKey, true)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, config.enterMsg[gender])
	end

	return true
end

thermalFountain:aid(GLOBAL_SYSTEMS.thermal.male, GLOBAL_SYSTEMS.thermal.female)
thermalFountain:register()
