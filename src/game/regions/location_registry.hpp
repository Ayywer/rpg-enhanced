#pragma once

#include "game/region.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace rpg {

class ItemDatabase;
class Player;

struct RegionLocationContext {
    std::shared_ptr<Player> player;
    const ItemDatabase& item_database;
};

class RegionLocationRegistry {
public:
    using Handler = std::function<void(const RegionLocationContext&,
                                       const RegionLocation&)>;

    static void register_builtin_function(std::string function_id,
                                          Handler handler);
    static bool has_builtin_function(const std::string& function_id);
    void register_function(std::string function_id, Handler handler);
    bool contains(const std::string& function_id) const;
    void execute(const std::string& function_id,
                 const RegionLocationContext& context,
                 const RegionLocation& location) const;

private:
    using HandlerMap = std::unordered_map<std::string, Handler>;
    static HandlerMap& builtin_handlers();

    std::unordered_map<std::string, Handler> m_Handlers;
};

} // namespace rpg