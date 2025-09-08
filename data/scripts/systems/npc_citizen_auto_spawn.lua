-- data/scripts/npc/npc_citizen_auto_spawn.lua

local MALE_NAMES = {
    "Haruto", "Ren", "Takeshi", "Daichi", "Yamato", "Taro", "Shinji", "Hayato",
    "Ryu", "Katsuo", "Hikaru", "Makoto", "Sora", "Naoki", "Yuji", "Akira",
    "Takumi", "Masato", "Itsuki", "Kaito", "Satoshi", "Jiro", "Fumio", "Riku",
    "Keita", "Hiroshi", "Kazuma", "Minato", "Yusuke", "Sho"
}
local FEMALE_NAMES = {
    "Aiko", "Yumi", "Hana", "Kaori", "Miyu", "Reina", "Aya", "Emi",
    "Kanna", "Sayuri", "Nozomi", "Saki", "Mai", "Nana", "Haruka", "Yua",
    "Chihiro", "Rina", "Mei", "Kiyomi", "Hikari", "Natsumi", "Mio", "Ayame",
    "Kana", "Riko", "Suzume", "Misaki", "Yoshino", "Nanami"
}

local LAST_NAMES = {
    "Fujimoto", "Kobayashi", "Nakamura", "Takahashi", "Matsumoto", "Tanaka", "Saito", "Mori",
    "Sugiyama", "Ando", "Kawaguchi", "Shimada", "Yoshida", "Ishikawa", "Hoshino", "Noguchi",
    "Sakurai", "Fujita", "Kimura", "Sakamoto", "Shibata", "Yamada", "Okamoto", "Suzuki",
    "Nakajima", "Morita", "Kawasaki", "Hirata", "Yamamoto", "Honda", "Kaneko", "Shinohara",
    "Miyazaki", "Murakami", "Kuroda", "Iida", "Takayama", "Kishimoto", "Abe", "Endo",
    "Ueda", "Nomura", "Watanabe", "Hamada", "Ono", "Ogawa", "Hasegawa", "Inoue",
    "Hayashi", "Kondo", "Miyamoto", "Hashimoto", "Sasaki", "Fujii", "Takeuchi", "Matsuda",
    "Yoshikawa", "Tsuchiya", "Kinoshita", "Otani"
}

local POSITIONS = {
    {x = 1227, y = 953, z = 7}, {x = 1233, y = 977, z = 7},
    {x = 1249, y = 964, z = 7}, {x = 1222, y = 988, z = 7},
    {x = 1275, y = 964, z = 7}, {x = 1262, y = 988, z = 7},
    {x = 1294, y = 964, z = 7}, {x = 1283, y = 989, z = 7},
    {x = 1218, y = 1004, z = 7}, {x = 1261, y = 1035, z = 7},
    {x = 1285, y = 1033, z = 7}, {x = 1262, y = 1045, z = 7},
    {x = 1316, y = 1032, z = 7}, {x = 1321, y = 1015, z = 7},
    {x = 1282, y = 1052, z = 7}, {x = 1262, y = 1065, z = 7},
    {x = 1222, y = 1062, z = 7}, {x = 1205, y = 1028, z = 7},
    {x = 1218, y = 1020, z = 7}, {x = 1194, y = 993, z = 7},
    {x = 1213, y = 954, z = 7}, {x = 1338, y = 991, z = 7},
    {x = 1339, y = 1009, z = 7}, {x = 1333, y = 1030, z = 7},
    {x = 1259, y = 979, z = 7}, {x = 1276, y = 987, z = 7},
    {x = 1287, y = 987, z = 7}, {x = 1301, y = 986, z = 7}
}
local GREETS = {
    "Olá, jovem ninja!", "Bem-vindo à vila!", "Tenha um bom dia!", "Traga honra ao seu clã!"
}
local GREETS = {
    "Welcome to our village, young shinobi!",
    "Stay alert, danger can come from anywhere.",
    "Train hard and protect your comrades.",
    "May your chakra flow strong today!",
    "The Hokage watches over us all.",
    "Honor your clan and ancestors.",
    "Don’t underestimate the power of teamwork.",
    "Beware of enemy ninjas beyond the forest.",
    "I heard strange things are happening near the river.",
    "Good luck on your next mission!",
    "You remind me of my youth as a genin.",
    "The market has fresh dango today!",
    "A true ninja never reveals all his secrets.",
    "Keep your weapons sharp and your mind sharper.",
    "The exams are coming soon, be prepared.",
    "Sometimes the greatest battles are within.",
    "Welcome, stranger. Are you a traveler or a shinobi?",
    "Take care in the training grounds. Accidents happen.",
    "Even a small spark can start a great fire.",
    "Respect your sensei and you’ll go far.",
    "Shadow clones are not allowed in the shop!",
    "Don’t forget to rest and recover your chakra.",
    "A smile can be a ninja’s best disguise.",
    "Patrol duty is important, even if it’s boring.",
    "Did you hear about the new jutsu from the east?",
    "Hard work beats talent when talent doesn’t work hard.",
    "Never turn your back on your friends.",
    "Keep an eye out for rogue shinobi.",
    "Wind style jutsu are powerful in this region.",
    "Legends say the First Hokage once trained here.",
    "Every day is a chance to get stronger.",
    "Don’t be late for your squad meeting.",
    "Sometimes, silence is the best answer.",
    "Even the quietest ninja can make a difference.",
    "Do you know any good ramen shops?",
    "The stars look brightest from the village rooftops.",
    "Don’t drop your guard, even during peace.",
    "A true shinobi helps those in need.",
    "You look like you’ve seen a ghost!",
    "Fate brings us all together here.",
    "If you see a suspicious person, report it to the guards.",
    "Chakra control is the key to mastering jutsu.",
    "Don’t overdo it, rest is part of training.",
    "Never give up, that is a ninja’s way.",
    "The best defense is sometimes a swift retreat.",
    "Are you here for a mission?",
    "Show respect to the elders of the village.",
    "Some scars are proof of courage.",
    "Watch out for traps in the forest paths.",
    "Today’s challenges make tomorrow’s heroes.",
    "Remember, friends can beco"
}

local NPC_QUOTES = {
    "The village feels peaceful today.",
    "Did you finish your chakra training?",
    "I could go for a hot bowl of ramen.",
    "I wonder what mission I’ll get next.",
    "The Hokage seems busy these days.",
    "Rumor has it there’s a new S-rank mission.",
    "My kunai needs sharpening again.",
    "I envy the ANBU—they’re so mysterious.",
    "Shadow clone practice is harder than it looks.",
    "I wish I had a powerful kekkei genkai.",
    "The market is bustling with travelers.",
    "I heard strange noises last night.",
    "The exams are coming—everyone is nervous.",
    "Hard work always pays off for a ninja.",
    "My friend failed the last genin test.",
    "Sometimes I dream of becoming Hokage.",
    "A ninja’s true strength comes from their heart.",
    "Did you see that lightning last night?",
    "I love watching the sunrise over the village walls.",
    "Rain always makes my chakra feel sluggish.",
    "Someone left shuriken stuck in my door again.",
    "Meditation helps me focus my chakra.",
    "The training grounds are crowded today.",
    "Even sensei needs to rest sometimes.",
    "Genjutsu gives me a headache.",
    "I heard a jonin mastered a new fire style jutsu.",
    "Stealth missions are the most challenging.",
    "Did you see the new equipment at the shop?",
    "I’m saving up for a better summoning contract.",
    "My pet toad keeps escaping.",
    "Losing is just another chance to learn.",
    "Sometimes, a good disguise is all you need.",
    "One day I’ll travel to all the hidden villages.",
    "Healing jutsu take a lot of concentration.",
    "Training in the rain builds endurance.",
    "There’s a festival coming soon!",
    "My little brother wants to be a ninja, too.",
    "Sometimes I wish I could just be a civilian.",
    "I lost my forehead protector... again.",
    "You can never have too many kunai.",
    "My teacher says patience is a ninja’s greatest weapon.",
    "I once met a ninja from the Mist Village.",
    "I heard the elders are planning something big.",
    "Sometimes the best jutsu is a friendly smile.",
    "Every scar tells a story.",
    "Summoning animals is harder than it looks.",
    "My favorite time is dusk in the village.",
    "I wish I was better at taijutsu.",
    "I’m practicing a new hand sign sequence.",
    "My friends bet I can’t climb the tallest tree.",
    "I got lost in the forest yesterday.",
    "Wind style is strong against lightning.",
    "Even a small mistake can be fatal for a ninja.",
    "Teamwork is what saved us last mission.",
    "I admire the medical ninjas—they’re so skilled.",
    "Sometimes I train until sunrise.",
    "Sensei says I should smile more.",
    "One day I’ll have my own students.",
    "I wonder what the future holds for our village.",
    "My chakra feels a bit off today.",
    "Never turn your back on your comrades."
}

local FAREWELLS = {
    "Stay safe out there!",
    "May the spirits guide your path.",
    "Don’t forget to train your chakra.",
    "Return victorious, shinobi!",
    "See you on your next mission.",
    "Farewell, and watch your back.",
    "Take care, young ninja.",
    "Come back if you need anything.",
    "Protect the village!",
    "May your journey be swift.",
    "Don’t let your guard down.",
    "I’ll be waiting for your return.",
    "Stay sharp!",
    "May fortune favor you.",
    "Until next time, friend.",
    "Keep your friends close.",
    "The Hokage trusts you.",
    "Bring honor to your clan.",
    "Safe travels!",
    "The village awaits your stories.",
    "Don’t stray too far from the village.",
    "Rest when you can.",
    "Be careful in the forest.",
    "Train hard and come back stronger.",
    "Hope to see you again soon.",
    "Stay hidden when you must.",
    "Remember your training.",
    "Never give up, never surrender.",
    "Keep an eye on your surroundings.",
    "If you get lost, follow the lanterns.",
    "Don’t go alone into the night.",
    "Keep your kunai ready.",
    "Let me know if you need advice.",
    "You are always welcome here.",
    "Look out for suspicious people.",
    "May your jutsu never fail you.",
    "Take a rest at the inn if needed.",
    "Let the will of fire burn within you.",
    "Don’t take unnecessary risks.",
    "Come back soon!",
    "Don’t forget to visit your family.",
    "Watch out for traps.",
    "Keep your scrolls safe.",
    "If you need healing, visit the hospital.",
    "May your journey be peaceful.",
    "Bring good news on your return.",
    "Tell your sensei I said hello.",
    "Be wary of strangers.",
    "Hope you learn something new.",
    "Leave no comrade behind.",
    "Find your ninja way.",
    "Let the wind be at your back.",
    "Keep the village secret safe.",
    "Even a hero needs to rest.",
    "Protect those who cannot fight.",
    "You’ve made the village proud.",
    "I’ll see you at the next festival.",
    "Don’t let darkness cloud your heart.",
    "A true shinobi always returns.",
    "Our gates are always open for you."
}


local MALE_OUTFITS = {1089, 1090, 1093, 1097, 1100}
local FEMALE_OUTFITS = {1098, 1095, 1091, 1096, 1100}
local CLAN_THEMES = {
    ["Uchiha"] = {male = 133, female = 144},
    ["Hyuuga"] = {male = 134, female = 145}
}
local SHOP_ITEMS = {
    {itemName = "kunai", clientId = 1001, buy = 20},
    {itemName = "shuriken", clientId = 1002, buy = 15, sell = 7},
    {itemName = "bento box", clientId = 1003, buy = 25},
    {itemName = "ramen", clientId = 1004, buy = 10, sell = 2},
    {itemName = "soldier pill", clientId = 1005, buy = 50}
}
local MALE_CITIZENS = 30
local FEMALE_CITIZENS = 30
local SELLER_CHANCE = 0.30

local usedNames, usedPositions = {}, {}

local function getUniqueNpcName(gender)
    local tries = 0
    while tries < 1000 do
        local first = (gender == "male" and MALE_NAMES or FEMALE_NAMES)[math.random(#(gender == "male" and MALE_NAMES or FEMALE_NAMES))]
        local last = LAST_NAMES[math.random(#LAST_NAMES)]
        local name = first .. " " .. last
        if not usedNames[name] then
            usedNames[name] = true
            return first, last, name
        end
        tries = tries + 1
    end
    error("Could not generate unique name for NPC! List exhausted?")
end

local function getUniquePosition()
    local tries = 0
    while tries < 1000 do
        local pos = POSITIONS[math.random(#POSITIONS)]
        local key = pos.x .. "-" .. pos.y .. "-" .. pos.z
        if not usedPositions[key] then
            usedPositions[key] = true
            return pos
        end
        tries = tries + 1
    end
    -- Em vez de error, retorna nil!
    return nil
end


local function getRandomOutfit(gender, last)
    local theme = CLAN_THEMES[last]
    if theme then
        return {
            lookType = theme[gender],
            lookHead = math.random(1, 132),
            lookBody = math.random(2, 132),
            lookLegs = math.random(3, 132),
            lookFeet = math.random(4, 132),
            lookAddons = math.random(0, 3)
        }
    end
    local lookType = (gender == "male" and MALE_OUTFITS or FEMALE_OUTFITS)[math.random(#(gender == "male" and MALE_OUTFITS or FEMALE_OUTFITS))]
    return {
        lookType = lookType,
        lookHead = math.random(1, 132),
        lookBody = math.random(2, 132),
        lookLegs = math.random(3, 132),
        lookFeet = math.random(4, 132),
        lookAddons = math.random(0, 3)
    }
end

local function getRandomShop()
    local shop = {}
    local count = math.random(2, math.min(4, #SHOP_ITEMS))
    local pool = {}
    for i = 1, #SHOP_ITEMS do table.insert(pool, i) end
    for i = 1, count do
        local idx = table.remove(pool, math.random(#pool))
        table.insert(shop, SHOP_ITEMS[idx])
    end
    return shop
end

local function spawnRandomNpc(position, gender)
    local first, last, name = getUniqueNpcName(gender)
    local outfit = getRandomOutfit(gender, last)
    local greet = GREETS[math.random(#GREETS)]
    local farewell = FAREWELLS[math.random(#FAREWELLS)]
    local isSeller = (math.random() < SELLER_CHANCE)
    local npcDesc = isSeller and name .. ". Um vendedor ambulante" or name .. ". Um cidadão de Konoha"
    local npcShop = isSeller and getRandomShop() or nil

    -- Criar o tipo dinâmico do NPC
    local npcType = Game.createNpcType(name)
    local npcConfig = {}
    npcConfig.name = name
    npcConfig.description = npcDesc
    npcConfig.health = 100
    npcConfig.maxHealth = 100
    npcConfig.walkInterval = math.random(1000, 4000)
    npcConfig.walkRadius = math.random(10, 20)
    npcConfig.outfit = outfit
    npcConfig.flags = {
        floorchange = true,
    }
    npcConfig.voices = {
        interval = 10000,
        chance = 40,
        { text = NPC_QUOTES[math.random(#NPC_QUOTES)] }
    }
    npcConfig.greetMessage = greet
    npcConfig.farewellMessage = farewell
    if isSeller then npcConfig.shop = npcShop end

    -- -- NpcHandler & KeywordHandler -- --
    local keywordHandler = KeywordHandler:new()
    local npcHandler = NpcHandler:new(keywordHandler)
    npcHandler:addModule(FocusModule:new(), name, true, true, true)

    -- Saudação customizada
        npcHandler:setCallback(CALLBACK_GREET, function(npc, player)
        if isSeller then
            npcHandler:setMessage(MESSAGE_GREET, "Olá, jovem ninja! Se quiser algo especial, é só falar.")
        else
            npcHandler:setMessage(MESSAGE_GREET, greet)
        end
        return true
    end)

    -- Mensagem de "bye"
    npcHandler:setMessage(MESSAGE_FAREWELL, farewell)
    -- Mensagem de walkaway
    npcHandler:setMessage(MESSAGE_WALKAWAY, "Até mais!")

    -- Adiciona o classic "hi"
    keywordHandler:addKeyword({"hi", "oi", "ola"}, function(npc, player)
        npcHandler:greet(player)
    end, {npcHandler = npcHandler})

    -- Shop handlers
    if isSeller then
        npcType.onBuyItem = function(npc, player, itemId, subType, amount, ignore, inBackpacks, totalCost)
            npc:sellItem(player, itemId, amount, subType, 0, ignore, inBackpacks)
        end
        npcType.onSellItem = function(npc, player, itemId, subtype, amount, ignore, name, totalCost)
            player:sendTextMessage(MESSAGE_TRADE, string.format("Sold %ix %s for %i gold.", amount, name, totalCost))
        end
        npcType.onCheckItem = function(npc, player, clientId, subType) end
    end

    -- Vincula o handler principal
    npcType.onAppear = function(npc, creature) npcHandler:onAppear(npc, creature) end
    npcType.onDisappear = function(npc, creature) npcHandler:onDisappear(npc, creature) end
    npcType.onMove = function(npc, creature, fromPosition, toPosition) npcHandler:onMove(npc, creature, fromPosition, toPosition) end
    npcType.onSay = function(npc, player, messageType, message) npcHandler:onSay(npc, player, messageType, message) end
    npcType.onThink = function(npc, interval) npcHandler:onThink(npc, interval) end

    npcType:register(npcConfig)
    local npc = Game.createNpc(name, position)
    if npc then
        npc:setMasterPos(position)
    end
end

function CitizenOnStartup()
    for i = 1, MALE_CITIZENS do
        local pos = getUniquePosition()
        if pos then
            spawnRandomNpc(pos, "male")
        end
    end

    for i = 1, FEMALE_CITIZENS do
        local pos = getUniquePosition()
        if pos then
            spawnRandomNpc(pos, "female")
        end
    end
end
