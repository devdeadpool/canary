local setting = {
	[16198] = SKILL_BUKIJUTSU,
	[16199] = SKILL_AXE,
	[16200] = SKILL_FUINJUTSU,
	[16201] = SKILL_GENJUTSU,
	[16202] = SKILL_NINJUTSU,
}

local skillTrainer = Action()

function skillTrainer.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local skill = setting[item:getId()]
	if not player:isPremium() then
		player:sendCancelMessage(RETURNVALUE_YOUNEEDPREMIUMACCOUNT)
		return true
	end

	if player:isPzLocked() then
		return false
	end

	player:setOfflineTrainingSkill(skill)
	player:remove(false)
	return true
end

for index, value in pairs(setting) do
	skillTrainer:id(index)
end

skillTrainer:register()
