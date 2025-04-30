function Player.canAdvanceStage(self)
	local currentStage = self:getStage()
	local nextStage = currentStage + 1
	return self:hasMissionRequirement()
end

function resetMissions(player)
	local kv = player:kv():scoped("mission")
	local keys = kv:keys()
	for _, key in ipairs(keys) do
		kv:remove(key)
	end
end

local talkaction = TalkAction("/teste")

function talkaction.onSay(player, words, param)
	local args = param:split(" ")

	local action = args[1]
	if not action then
		player:sendCancelMessage("Use: /teste [set|info|avancar|objetivo]")
		return false
	end

	if action == "set" then
		local missionId = tonumber(args[2])
		local stage = tonumber(args[3])
		if not missionId or not stage then
			player:sendCancelMessage("Uso: /teste set [missionId] [stage]")
			return false
		end

		player:setMissionId(missionId)
		player:setMissionStage(stage)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Missão ativa definida para ID: " .. missionId .. ", estágio: " .. stage)

	elseif action == "leo" then
		print(player:sendMissionInfo())
	elseif action == "reset" then
		resetMissions(player)
		
	elseif action == "info" then
		player:sendMissionInfo()


	elseif action == "avancar" then
		local currentStage = player:getStage()
		player:sendTextMessage(MESSAGE_LOOK, "Estágio atual: " .. currentStage)

		if not player:canAdvanceStage() then
			player:sendTextMessage(MESSAGE_LOOK, "❌ Você NÃO cumpre os requisitos para o próximo estágio.")
			return false
		end

		player:setMissionStage(currentStage + 1)
		player:sendTextMessage(MESSAGE_LOOK, "✅ Você avançou para o estágio " .. (currentStage + 1))

	elseif action == "objetivo" then
		local index = tonumber(args[2])
		if not index then
			player:sendCancelMessage("Uso: /teste objetivo [index]")
			return false
		end

		if player:isMissionObjectiveCompleted(index) then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "✅ Objetivo " .. index .. " completo.")
		else
			player:sendTextMessage(MESSAGE_LOOK, "❌ Objetivo " .. index .. " ainda não completo.")
		end

	else
		player:sendCancelMessage("Comando desconhecido: " .. action)
	end

	return false
end

talkaction:separator(" ")
talkaction:groupType("god")
talkaction:register()
