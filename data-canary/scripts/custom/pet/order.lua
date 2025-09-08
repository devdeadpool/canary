local petOrder = Action()

function petOrder.onUse(player, item, fromPosition, target, toPosition, isHotkey)
    local summons = player:getSummons()
    if #summons == 0 then
        player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Você não possui um pet invocado.")
        return true
    end

    local pet = summons[1]
    if not pet or not pet:isCreature() then
        player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Pet inválido.")
        return true
    end

    local dest = Position(toPosition)
    local path = pet:getPathTo(dest, 0, 1, true, true, 100)
    

    if type(path) == "table" and #path > 0 then
        player:movePetTo(dest)
          print("Passos: " .. #path)
    else
        player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Não foi possível gerar um caminho até essa posição.")
    end

    return true
end

petOrder:id(110) -- Use o ID do item que dá a ordem
petOrder:allowFarUse(true)
petOrder:register()
