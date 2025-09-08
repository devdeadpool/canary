local talkaction = TalkAction("/teste")

function talkaction.onSay(player, words, param)
	local args = param:split(" ")

	local action = args[1]
	if not action then
		player:sendCancelMessage("Use: /teste [set|stage|ativar|desativar|toggle|info|ativo|jutsus|slots]")
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

	elseif action == "jutsus" then
		local jutsus = player:getCopiedJutsus()
		if #jutsus == 0 then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Nenhum jutsu copiado.")
		else
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Jutsus copiados: " .. table.concat(jutsus, ", "))
		end

	elseif action == "shader" then
		player:setShader("Outfit - cyclopedia-black")
	elseif action == "nick" then
		player:setPetName(1, "Charuto Uzukrak")

	elseif action == "petinfo" then
 		local info = player:getPetInfo(1)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, info)

	elseif action == "addpet" then
		player:addPet("Akamaru")



	elseif action == "pet" then
		player:togglePet(1)
	elseif action == "copy" then
	if player:hasCopiedJutsu("Eternal Winters") then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Você já copiou Eternal Winter.")
	else
		player:addCopiedJutsu("Eternal Winters")
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Você copiou Eternal Winter.")
	end
	elseif action == "slots" then
		local slots = player:getSharinganJutsuSlots()
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Você possui " .. slots .. " slot(s) de jutsus copiados.")

	else
		player:sendCancelMessage("Comando desconhecido: " .. action)
	end

	return false
end

talkaction:separator(" ")
talkaction:groupType("god")
talkaction:register()
