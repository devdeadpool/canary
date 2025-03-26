local talkaction = TalkAction("/at")

function talkaction.onSay(player, words, param)
	Game.createItem(ITEM_SHARINGAN, 1, player:getPosition())
	local split = param:split(" ")

	if not split[1] then
		player:sendCancelMessage("Usage: /at get <attribute> | /at set <attribute> <value> | /at cost <attribute> | /at points | /at reset")
		return false
	end

	local action = split[1]:lower()

	-- Lista de atributos disponíveis
	local attributes = {
		strength = 0,
		agility = 1,
		intelligence = 2,
		energy = 3,
		focus = 4,
		perception = 5,
		determination = 6,
	}

	if action == "get" then
		local attribute = split[2] and split[2]:lower()
		if not attribute or not attributes[attribute] then
			player:sendCancelMessage("Invalid attribute. Use: strength, agility, intelligence, energy, focus, perception, determination.")
			return false
		end

		local value = player:getStatusAttribute(attributes[attribute])
		player:sendTextMessage(MESSAGE_GAME_HIGHLIGHT, "Your " .. attribute .. " is: " .. value)
		return true
	end

	if action == "set" then
		local attribute = split[2] and split[2]:lower()
		local value = tonumber(split[3])

		if not attribute or not attributes[attribute] or not value or value < 1 then
			player:sendCancelMessage("Usage: /at set <attribute> <value>")
			return false
		end

		local currentValue = player:getStatusAttribute(attributes[attribute])
		local cost = player:getStatusPointCost(attributes[attribute])

		if player:getStatusPoints() < cost then
			player:sendCancelMessage("Not enough status points! You need " .. cost .. " points.")
			return false
		end

		-- Chama a função do C++ para aplicar os pontos corretamente
		player:setStatusAttribute(attributes[attribute], value)

		player:sendTextMessage(MESSAGE_GAME_HIGHLIGHT, "Your " .. attribute .. " is now: " .. (currentValue + value) .. " (Cost: " .. cost .. " points)")
		return true
	end

	if action == "cost" then
		local attribute = split[2] and split[2]:lower()
		if not attribute or not attributes[attribute] then
			player:sendCancelMessage("Invalid attribute. Use: strength, agility, intelligence, energy, focus, perception, determination.")
			return false
		end

		-- Obtém o custo progressivo do próximo ponto
		local cost = player:getStatusPointCost(attributes[attribute])
		player:sendTextMessage(MESSAGE_GAME_HIGHLIGHT, "The next point in " .. attribute .. " will cost: " .. cost .. " status points.")
		return true
	end

	if action == "points" then
		local availablePoints = player:getStatusPoints()
		player:sendTextMessage(MESSAGE_GAME_HIGHLIGHT, "You have " .. availablePoints .. " status points available.")
		return true
	end

	if action == "reset" then
		player:resetStatusAttributes()
		player:sendTextMessage(MESSAGE_GAME_HIGHLIGHT, "All your attributes have been reset!")
		return true
	end

	player:sendCancelMessage("Usage: /at get <attribute> | /at set <attribute> <value> | /at cost <attribute> | /at points | /at reset")
	return false
end

talkaction:separator(" ")
talkaction:groupType("god")
talkaction:register()
