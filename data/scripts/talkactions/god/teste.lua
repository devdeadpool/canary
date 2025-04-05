local talkaction = TalkAction("/teste")

function talkaction.onSay(player, words, param)
	player:sendCombatStatsInfo()
	return false
end

talkaction:separator(" ")
talkaction:groupType("god")
talkaction:register()
