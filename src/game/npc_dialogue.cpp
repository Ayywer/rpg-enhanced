#include "region.hpp"

#include <json-c/json.h>

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace rpg {
namespace {

using JsonObject = std::unique_ptr<json_object, decltype(&json_object_put)>;

json_object* require_member(json_object* object, const char* name,
                            const std::string& context) {
    json_object* value = nullptr;
    if (!json_object_object_get_ex(object, name, &value)) {
        throw std::runtime_error("Missing '" + std::string(name) + "' in " + context);
    }
    return value;
}

std::string read_string(json_object* object, const char* name,
                        const std::string& context) {
    json_object* value = require_member(object, name, context);
    if (!json_object_is_type(value, json_type_string)) {
        throw std::runtime_error("Expected string '" + std::string(name) +
                                 "' in " + context);
    }
    return json_object_get_string(value);
}

std::unordered_map<std::string, std::vector<NpcDialogueOption>> load_registry(
    const std::string& database_path) {
    JsonObject root(json_object_from_file(database_path.c_str()), json_object_put);
    if (!root || !json_object_is_type(root.get(), json_type_object)) {
        throw std::runtime_error("Unable to load NPC dialogue database: " +
                                 database_path);
    }

    json_object* npcs = require_member(root.get(), "npcs", database_path);
    if (!json_object_is_type(npcs, json_type_array)) {
        throw std::runtime_error("Expected 'npcs' array in " + database_path);
    }

    std::unordered_map<std::string, std::vector<NpcDialogueOption>> registry;
    const std::size_t npc_count = json_object_array_length(npcs);
    for (std::size_t npc_index = 0; npc_index < npc_count; ++npc_index) {
        json_object* npc = json_object_array_get_idx(npcs, npc_index);
        const std::string context = "npcs[" + std::to_string(npc_index) + "]";
        if (!json_object_is_type(npc, json_type_object)) {
            throw std::runtime_error("Expected object in " + context);
        }

        const std::string npc_id = read_string(npc, "id", context);
        if (npc_id.empty()) {
            throw std::runtime_error("NPC ID cannot be empty in " + context);
        }
        json_object* options = require_member(npc, "options", context);
        if (!json_object_is_type(options, json_type_array)) {
            throw std::runtime_error("Expected options array in " + context);
        }

        std::vector<NpcDialogueOption> dialogue_options;
        std::unordered_set<std::string> option_ids;
        const std::size_t option_count = json_object_array_length(options);
        dialogue_options.reserve(option_count);
        for (std::size_t option_index = 0; option_index < option_count;
             ++option_index) {
            json_object* option = json_object_array_get_idx(options, option_index);
            const std::string option_context =
                context + ".options[" + std::to_string(option_index) + "]";
            if (!json_object_is_type(option, json_type_object)) {
                throw std::runtime_error("Expected object in " + option_context);
            }

            NpcDialogueOption dialogue_option{
                read_string(option, "id", option_context),
                read_string(option, "label", option_context),
                read_string(option, "message", option_context),
            };
            if (dialogue_option.id.empty() || dialogue_option.label.empty() ||
                dialogue_option.message.empty() ||
                !option_ids.insert(dialogue_option.id).second) {
                throw std::runtime_error("Invalid or duplicate dialogue option in " +
                                         option_context);
            }
            dialogue_options.push_back(std::move(dialogue_option));
        }

        if (!registry.emplace(npc_id, std::move(dialogue_options)).second) {
            throw std::runtime_error("Duplicate NPC ID '" + npc_id + "'");
        }
    }
    return registry;
}

const std::unordered_map<std::string, std::vector<NpcDialogueOption>>& npc_registry() {
    static const auto registry = load_registry(RPG_NPC_DIALOGUE_DATABASE_PATH);
    return registry;
}

} // namespace

const std::vector<NpcDialogueOption>& NpcDialogueRegistry::options_for(
    const std::string& npc_id) {
    static const std::vector<NpcDialogueOption> empty;
    const auto& registry = npc_registry();
    const auto it = registry.find(npc_id);
    return it == registry.end() ? empty : it->second;
}

} // namespace rpg