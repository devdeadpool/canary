/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2024 OpenTibiaBR
 * Repository: https://github.com/opentibiabr/canary
 */

 #include "creatures/players/missions/mission_manager.hpp"
 #include "creatures/monsters/monster.hpp"
 #include "creatures/monsters/monsters.hpp"
 #include "lua/functions/events/action_functions.hpp"
 #include "lua/creature/actions.hpp"
 #include "game/game.hpp"
 #include "config/configmanager.hpp"
 #include "lib/di/container.hpp"
 #include "utils/pugicast.hpp"
 #include "utils/tools.hpp"
 #include "kv/kv.hpp"
 
 MissionManager &MissionManager::getInstance() {
     static MissionManager instance;
     return instance;
 }
 
 bool MissionManager::loadFromXML() {
     missions.clear();
 
     const std::string path = g_configManager().getString(CORE_DIRECTORY) + "/XML/missions/mission_list.xml";
 
     pugi::xml_document doc;
     pugi::xml_parse_result result = doc.load_file(path.c_str());
     if (!result) {
         printXMLError(__FUNCTION__, path, result);
         return false;
     }
 
     for (const auto &missionNode : doc.child("missions").children("mission")) {
         const std::string filename = missionNode.attribute("file").as_string();
         if (filename.empty()) {
             g_logger().warn("[MissionManager] Skipping mission entry with missing file attribute.");
             continue;
         }
 
         loadMissionFile(g_configManager().getString(CORE_DIRECTORY) + "/XML/" + filename);
         //registerKillEventsFromObjectives();
         registerUseActionsFromObjectives();
     }
 
     g_logger().info("[MissionManager] Loaded {} mission(s).", missions.size());
     return true;
 }
 
 /* void MissionManager::loadMissionList() {
     pugi::xml_document doc;
     if (!doc.load_file("data/missions/mission_list.xml")) {
         std::cerr << "[MissionManager] Failed to load mission_list.xml" << std::endl;
         return;
     }
 
     for (auto node : doc.child("missions").children("mission")) {
         uint32_t id = node.attribute("id").as_uint();
         std::string file = node.attribute("file").as_string();
         loadMissionFile("data/missions/" + file);
     }
 }
  */
 void MissionManager::loadMissionFile(const std::string &filepath) {
     pugi::xml_document doc;
     pugi::xml_parse_result result = doc.load_file(filepath.c_str());
     if (!result) {
         printXMLError(__FUNCTION__, filepath, result);
         return;
     }
 
     const auto root = doc.child("mission");
     if (!root) {
         g_logger().warn("[MissionManager] Missing <mission> root in '{}'", filepath);
         return;
     }
 
     Mission mission;
     pugi::xml_attribute attr;
 
     if (!(attr = root.attribute("id"))) {
         g_logger().warn("[MissionManager] Missing id for mission in '{}'", filepath);
         return;
     }
     mission.id = pugi::cast<uint32_t>(attr.value());
 
     if (!(attr = root.attribute("name"))) {
         g_logger().warn("[MissionManager] Missing name for mission '{}'", filepath);
         return;
     }
     mission.name = attr.as_string();
 
     mission.repeatable = std::string(root.attribute("repeatable").as_string()) == "true";
     mission.description = root.child_value("description");
 
     for (const auto &stageNode : root.child("stages").children("stage")) {
         MissionStage stage;
 
         if (!(attr = stageNode.attribute("id"))) {
             g_logger().warn("Missing stage ID in mission '{}'", mission.name);
             continue;
         }
         stage.id = pugi::cast<uint32_t>(attr.value());
 
         stage.name = stageNode.attribute("name").as_string();
 
         if (const auto reqs = stageNode.child("requirements")) {
             for (const auto &kv : reqs.children("kv")) {
                 std::string key = kv.attribute("key").as_string();
                 std::string value = kv.attribute("value").as_string();
                 if (!key.empty()) {
                     stage.kvRequirements.emplace_back(key, value);
                 }
             }
         }
 
         for (const auto &obj : stageNode.child("objectives").children()) {
             MissionObjective objective;
             const std::string tag = obj.name();
         
             if (tag == "kill") {
                 objective.type = MissionObjectiveType::KILL;
                 objective.param = obj.attribute("monster").as_string();
             } else if (tag == "collect") {
                 objective.type = MissionObjectiveType::COLLECT;
                 objective.param = obj.attribute("itemid").as_string();
             } else if (tag == "investigate") {
                 objective.type = MissionObjectiveType::INVESTIGATE;
                 objective.param = obj.attribute("position").as_string();
                 objective.description = obj.attribute("description").as_string();
             } else if (tag == "sabotage") {
                 objective.type = MissionObjectiveType::INVESTIGATE;
                 objective.param = obj.attribute("itemid").as_string();
                 objective.description = obj.attribute("position").as_string(); // se quiser guardar pos também
             } else if (tag == "go") {
                 objective.type = MissionObjectiveType::INVESTIGATE; // ALIAS
                 objective.param = obj.attribute("position").as_string();
                 objective.description = obj.attribute("description").as_string();
             } else if (tag == "use") {
                 objective.type = MissionObjectiveType::SABOTAGE; // ALIAS
                 objective.param = obj.attribute("itemid").as_string();
                 objective.description = obj.attribute("position").as_string();
             } else if (tag == "talk") {
                 objective.type = MissionObjectiveType::TALK;
                 objective.param = obj.attribute("npc").as_string();
             } else if (tag == "escort") {
                 objective.type = MissionObjectiveType::ESCORT;
                 objective.param = obj.attribute("npc").as_string();
                 objective.description = obj.attribute("position").as_string(); // destino
             } else if (tag == "defend") {
                 objective.type = MissionObjectiveType::DEFEND;
                 objective.param = obj.attribute("npc").as_string();
             } else if (tag == "capture") {
                 objective.type = MissionObjectiveType::CAPTURE;
                 objective.param = obj.attribute("npc").as_string();
             } else if (tag == "survive") {
                 objective.type = MissionObjectiveType::SURVIVE;
                 objective.param = obj.attribute("seconds").as_string(); // armazenado como texto para simplificar
             } else {
                 g_logger().warn("Unknown objective type '{}' in mission '{}'", tag, mission.name);
                 continue;
             }
         
             objective.amount = obj.attribute("amount").as_uint(1);
             stage.objectives.emplace_back(std::move(objective));
         }
         
 
         for (const auto &reward : stageNode.child("rewards").children()) {
             MissionReward r;
             const std::string tag = reward.name();
 
             if (tag == "xp") {
                 r.type = MissionRewardType::XP;
                 r.value = reward.attribute("amount").as_uint();
             } else if (tag == "item") {
                 r.type = MissionRewardType::ITEM;
                 r.name = reward.attribute("id").as_string();
                 r.value = reward.attribute("count").as_uint(1);
             } else if (tag == "attributePoints") {
                 r.type = MissionRewardType::ATTR_POINTS;
                 r.value = reward.attribute("amount").as_uint();
             } else if (tag == "skill") {
                 r.type = MissionRewardType::SKILL_TRIES;
                 r.name = reward.attribute("name").as_string();
                 r.value = reward.attribute("tries").as_uint();
             } else if (tag == "perk") {
                 r.type = MissionRewardType::PERK;
                 r.name = reward.attribute("id").as_string();
             } else if (tag == "kv") {
                 r.type = MissionRewardType::KVE;
                 r.name = reward.attribute("key").as_string();
                 r.extraValue = reward.attribute("value").as_string();
             } else {
                 g_logger().warn("Unknown reward type '{}' in mission '{}'", tag, mission.name);
                 continue;
             }
 
             stage.rewards.emplace_back(std::move(r));
         }
 
         mission.stages.emplace_back(std::move(stage));
     }
 
     missions.emplace_back(std::move(mission));
 }
 
 const Mission* MissionManager::getMissionById(uint32_t id) const {
     for (const auto& mission : missions) {
         if (mission.id == id) {
             return &mission;
         }
     }
     return nullptr;
 }
 
 void MissionManager::setStage(Player& player, const std::string& missionName, uint32_t stage) const {
     player.kv()->scoped("mission")->scoped(missionName)->set("stage", static_cast<int>(stage));
 }
 
 /* uint32_t MissionManager::getStage(Player &player, const std::string &missionName) const {
     const auto value = player.kv()->scoped("mission")->scoped(missionName)->get("stage");
     return value ? static_cast<uint32_t>(value->get<int>()) : 0;
 }
  */
 
 
 bool MissionManager::hasRequirements(Player& player, const MissionStage& stage) const {
     for (const auto& [key, expected] : stage.kvRequirements) {
         auto kvRef = player.kv();
         std::string scopedKey = key;
         size_t dot;
         while ((dot = scopedKey.find('.')) != std::string::npos) {
             kvRef = kvRef->scoped(scopedKey.substr(0, dot));
             scopedKey = scopedKey.substr(dot + 1);
         }
         const auto current = kvRef->get(scopedKey);
         if (!current.has_value() || current->get<std::string>() != expected) {
             return false;
         }
     }
     return true;
 }
 /*  */
 bool MissionManager::isMissionObjectiveCompleted(Player& player, const MissionObjective& objective) const {
     auto missionKV = player.kv()->scoped("mission")->scoped("objectives");
 
     switch (objective.type) {
         case MissionObjectiveType::KILL:
         case MissionObjectiveType::COLLECT:
         case MissionObjectiveType::SABOTAGE: {
             const auto value = missionKV->get(objective.param);
             return value.has_value() && value->get<int>() >= static_cast<int>(objective.amount);
         }
         case MissionObjectiveType::INVESTIGATE:
         case MissionObjectiveType::TALK: {
             const auto value = missionKV->get(objective.param);
             return value.has_value() && value->get<bool>();
         }
         default:
             return false;
     }
 }
 
 
 // NOVAS FUNÇÕES DE MISSÃO ATIVA
 void MissionManager::setActiveMissionId(Player &player, uint32_t missionId) const {
     player.kv()->scoped("mission")->scoped("active")->set("id", static_cast<int>(missionId));
 }
 
 void MissionManager::setActiveMissionStage(Player &player, uint32_t stage) const {
     player.kv()->scoped("mission")->scoped("active")->set("stage", static_cast<int>(stage));
 }
 
 uint32_t MissionManager::getActiveMissionId(Player &player) const {
     const auto idValue = player.kv()->scoped("mission")->scoped("active")->get("id");
     return idValue ? static_cast<uint32_t>(idValue->get<int>()) : 0;
 }
 
 uint32_t MissionManager::getActiveMissionStageIndex(Player &player) const {
     const auto stageValue = player.kv()->scoped("mission")->scoped("active")->get("stage");
     return stageValue ? static_cast<uint32_t>(stageValue->get<int>()) : 0;
 }
 
 uint32_t MissionManager::getStage(Player &player) const {
     const MissionStage* stage = getActiveMissionStage(player);
     return stage ? stage->id : 0;
 }
 
 bool MissionManager::isObjectiveCompleted(Player &player, uint32_t objectiveIndex) const {
     const MissionStage* stage = getActiveMissionStage(player);
     if (!stage || objectiveIndex >= stage->objectives.size()) {
         return false;
     }
 
     return isMissionObjectiveCompleted(player, stage->objectives[objectiveIndex]);
 }
 
 const Mission* MissionManager::getActiveMission(Player &player) const {
     uint32_t id = getActiveMissionId(player);
     return getMissionById(id);
 }
 
 const MissionStage* MissionManager::getActiveMissionStage(Player &player) const {
     const Mission* mission = getActiveMission(player);
     if (!mission) {
         return nullptr;
     }
 
     uint32_t stageIndex = getActiveMissionStageIndex(player);
     if (stageIndex >= mission->stages.size()) {
         return nullptr;
     }
 
     return &mission->stages[stageIndex];
 }
 
 std::string MissionManager::getMissionStageInfoString(Player& player) const {
     const Mission* mission = getActiveMission(player);
     if (!mission) {
         return "Nenhuma missão ativa encontrada.";
     }
 
     uint32_t stageIndex = getActiveMissionStageIndex(player);
     if (stageIndex >= mission->stages.size()) {
         return "Você já concluiu essa missão.";
     }
 
     const auto& stage = mission->stages[stageIndex];
     std::string text;
 
     text += "· Mission: " + mission->name + "\n";
     text += "· Description: " + mission->description + "\n";
     text += "· Stage: " + std::to_string(stage.id) + " - " + stage.name + "\n";
 
     if (!stage.objectives.empty()) {
         text += "· Objectives:\n";
         for (const auto& obj : stage.objectives) {
             std::string desc = "- ";
             switch (obj.type) {
                 case MissionObjectiveType::KILL:
                     desc += "Kill " + std::to_string(obj.amount) + "x " + obj.param;
                     break;
                 case MissionObjectiveType::COLLECT:
                     desc += "Collect " + std::to_string(obj.amount) + "x item " + obj.param;
                     break;
                 case MissionObjectiveType::INVESTIGATE:
                     desc += "Investigate: " + obj.description;
                     break;
                 case MissionObjectiveType::SABOTAGE:
                     desc += "Sabotage/Use item: " + obj.param;
                     break;
                 case MissionObjectiveType::TALK:
                     desc += "Talk with " + obj.param;
                     break;
                 case MissionObjectiveType::ESCORT:
                     desc += "Escort " + obj.param + " até " + obj.description;
                     break;
                 case MissionObjectiveType::DEFEND:
                     desc += "Defende " + obj.param;
                     break;
                 case MissionObjectiveType::CAPTURE:
                     desc += "Capture (not kill) " + obj.param;
                     break;
                 case MissionObjectiveType::SURVIVE:
                     desc += "Survive for " + obj.param + " seconds";
                     break;
                 default:
                     desc += "[Objetivo desconhecido]";
                     break;
             }
             text += desc + "\n";
         }
     }
 
     if (!stage.rewards.empty()) {
         text += "· Rewards:\n";
         for (const auto& reward : stage.rewards) {
             std::string line = "- ";
             switch (reward.type) {
                 case MissionRewardType::XP:
                     line += std::to_string(reward.value) + " Experience";
                     break;
                 case MissionRewardType::ITEM:
                     line += std::to_string(reward.value) + "x item " + reward.name;
                     break;
             /* 	case MissionRewardType::ATTR_POINTS:
                     line += std::to_string(reward.value) + " pontos de atributo";
                     break; */
                 case MissionRewardType::SKILL_TRIES:
                     line += std::to_string(reward.value) + " tentativas de " + reward.name;
                     break;
                 /* case MissionRewardType::PERK:
                     line += "Perk: " + reward.name;
                     break; */
                 case MissionRewardType::KVE:
                     line += "Definir " + reward.name + " = " + reward.extraValue;
                     break;
             }
             text += line + "\n";
         }
     }
 
     return text;
 }
 
 std::shared_ptr<MonsterType> MissionManager::findMonsterTypeByName(const std::string& name) const {
     const std::string normalized = asLowerCaseString(name);
     for (const auto& [registeredName, mtype] : g_monsters().monsters) {
         if (asLowerCaseString(registeredName) == normalized) {
             return mtype;
         }
     }
     return nullptr;
 }
 
 void MissionManager::registerKillEventsFromObjectives() {
     std::unordered_set<std::string> monsterNames;
 
     // 1. Coletar todos os nomes de monstros usados em objetivos KILL
     for (const auto& mission : missions) {
         for (const auto& stage : mission.stages) {
             for (const auto& obj : stage.objectives) {
                 if (obj.type == MissionObjectiveType::KILL) {
                     monsterNames.insert(asLowerCaseString(obj.param));
                 }
             }
         }
     }
 
     // 2. Buscar o tipo real de monstro e registrar o evento
     for (const auto& name : monsterNames) {
         auto monsterTypeShared = findMonsterTypeByName(name);
         if (!monsterTypeShared) {
             g_logger().warn("[MissionManager] Monstro '{}' não encontrado para registrar evento 'MissionKill'", name);
             continue;
         }
 
         // 2.1. Registra no tipo
         monsterTypeShared->info.scripts.insert("MissionKill");
 
         // 2.2. Registra nas instâncias vivas com comparação por nome
         for (const auto& monster : g_game().getMonsters()) {
             if (monster && monster->getMonsterType()->name == monsterTypeShared->name) {
                 monster->registerCreatureEvent("MissionKill");
             }
         }
 
         g_logger().info("[MissionManager] Evento 'MissionKill' registrado no monstro '{}'", monsterTypeShared->name);
     }
 }
 
 void MissionManager::registerUseActionsFromObjectives() {
     for (const auto& mission : missions) {
         for (const auto& stage : mission.stages) {
             for (const auto& objective : stage.objectives) {
                 if (objective.type == MissionObjectiveType::SABOTAGE) {
                     uint16_t itemId = static_cast<uint16_t>(std::stoi(objective.param));
                     if (itemId == 0) {
                         g_logger().warn("[MissionManager] Falha ao registrar item de uso de missão: param inválido '{}'.", objective.param);
                         continue;
                     }
 
                     auto action = std::make_shared<Action>();
                     action->setItemIdsVector(itemId);
                     action->loadScriptId(); // Vincula o script atual
                     g_actions().registerLuaEvent(action); // Registra no sistema de Actions
                     g_logger().debug("[MissionManager] Registrado Action automático para ItemID {} (Objetivo de missão).", itemId);
                 }
             }
         }
     }
 }
 
 void MissionManager::onKill(Player& player, const std::string& monsterName) const {
     const MissionStage* stage = getActiveMissionStage(player);
     if (!stage) {
         return;
     }
 
     auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");
     std::string monsterKey = asLowerCaseString(monsterName);
 
     bool updated = false;
 
     for (const auto& objective : stage->objectives) {
         if (objective.type == MissionObjectiveType::KILL &&
             asLowerCaseString(objective.param) == monsterKey) {
 
             int current = objectivesKV->get(monsterKey).value_or(0);
 
             if (current >= static_cast<int>(objective.amount)) {
                 // Já completou esse objetivo
                 return;
             }
 
             int newValue = current + 1;
             objectivesKV->set(monsterKey, newValue);
 
             updated = true;
 
             player.sendTextMessage(MESSAGE_EVENT_ADVANCE,
                 fmt::format("Updated objective: {} ({}/{})", objective.param, newValue, objective.amount));
 
             break; // Encontrou e atualizou, não precisa continuar o loop
         }
     }
 
     if (updated) {
         checkStageCompletion(player);
     }
 }
 
 
 void MissionManager::onUse(Player& player, uint16_t itemId) const {
     auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");
     objectivesKV->set(std::to_string(itemId), true);
     checkStageCompletion(player);
 }
 
 void MissionManager::checkStageCompletion(Player& player) const {
    const Mission* mission = getMissionById(getActiveMissionId(player));
    if (!mission) {
        return;
    }

    uint32_t currentStageId = getActiveMissionStageIndex(player);
    if (currentStageId >= mission->stages.size()) {
        return;
    }

    const MissionStage& stage = mission->stages[currentStageId];
    if (stage.objectives.empty()) {
        return;
    }

    auto objectivesKV = player.kv()->scoped("mission")->scoped("objectives");

    for (const auto& objective : stage.objectives) {
        std::string key = asLowerCaseString(objective.param);

        // Coletar progresso atual
        int progress = objectivesKV->get(key).value_or(0);

        if (progress < static_cast<int>(objective.amount)) {
            // Ainda não completou esse objetivo
            return;
        }
    }

    // Se passou por todos os objetivos, avança
    uint32_t nextStage = currentStageId + 1;
    if (nextStage < mission->stages.size()) {
        setActiveMissionStage(player, nextStage);

        // Limpamos o progresso anterior
        player.kv()->scoped("mission")->scoped("objectives")->remove("");

        player.sendTextMessage(MESSAGE_EVENT_ADVANCE, "Stage completed! You've advanced to the next stage!");
    } else {
        // Missão totalmente concluída
        player.kv()->scoped("mission")->scoped("active")->set("completed", true);
        player.kv()->scoped("mission")->scoped("objectives")->remove("");

        player.sendTextMessage(MESSAGE_EVENT_ADVANCE, "Congratulations! You have completed the mission!");
    }
}
