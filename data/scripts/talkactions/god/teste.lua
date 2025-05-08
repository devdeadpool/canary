local talkaction = TalkAction("/teste")

function talkaction.onSay(player, words, param)
	local args = param:split(" ")

	local action = args[1]
	if not action then
		player:sendCancelMessage("Use: /teste [set|stage|ativar|desativar|toggle|info|ativo]")
		return false
	end

	if action == "stage" then
		local stage = tonumber(args[2])
		if not stage then
			player:sendCancelMessage("Uso: /teste stage [0-5]")
			return false
		end
		player:setSharinganStage(stage)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Stage do Sharingan definido para " .. stage)

	elseif action == "toggle" then
		player:toggleSharingan()

	elseif action == "ativar" then
		player:setSharinganActive(true)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Sharingan ativado via comando.")

	elseif action == "desativar" then
		player:setSharinganActive(false)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Sharingan desativado via comando.")

	elseif action == "info" then
		local stage = player:getSharinganStage()
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Stage atual do Sharingan: " .. stage)

	elseif action == "ativo" then
		if player:isSharinganActive() then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "✅ Sharingan está ATIVO.")
		else
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "❌ Sharingan está DESATIVADO.")
		end

--[[ 	-- mantém comandos antigos:
	elseif action == "reset" then
		resetMissions(player)

	elseif action == "graduation" then
		player:setGraduation("Genin")
 ]]
	else
		player:sendCancelMessage("Comando desconhecido: " .. action)
	end

	return false
end

talkaction:separator(" ")
talkaction:groupType("god")
talkaction:register()
