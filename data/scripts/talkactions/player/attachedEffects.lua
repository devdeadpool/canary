local mehah = {
    talkactions = {
        attacheffect = "!attacheffect",
        detachEffect = "!detacheffect",
        playerSetShader = "!playerSetShader",
        itemSetShader = "!itemSetShader",
        mapShader = "!mapShader"
    },
}

local events = {}


local attachEffect = TalkAction(mehah.talkactions.attacheffect)
function attachEffect.onSay(player, words, param, type)
	player:attachEffectById(tonumber(effect), false)
end
table.insert(events, attachEffect)

local detachEffect = TalkAction(mehah.talkactions.detachEffect)
function detachEffect.onSay(player, words, param, type)
    player:detachEffectById(tonumber(effect))
end
table.insert(events, detachEffect)

local setShader = TalkAction(mehah.talkactions.playerSetShader)
function setShader.onSay(player, words, param, type)
	player:setShader(shader)
end
table.insert(events, setShader)

local mapShader = TalkAction(mehah.talkactions.mapShader)
function mapShader.onSay(player, words, param, type)
	if player:getMapShader() ~= shader then
		player:setMapShader(shader, true)
	end
end
table.insert(events, mapShader)

for _, event in ipairs(events) do
    event:groupType("normal")
    event:register()
end
