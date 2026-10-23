cmake_minimum_required(VERSION 3.14)
project(rpg VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONC REQUIRED IMPORTED_TARGET json-c)

include(FetchContent)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_MODULES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS_FUZZER OFF CACHE BOOL "" FORCE)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(ftxui
	GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
	GIT_TAG v7.0.3
	GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(ftxui)

include(CheckCXXCompilerFlag)

# Enable flag if supported
function(enable_cxx_compiler_flag_if_supported flag)
	string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
	if(flag_already_set EQUAL -1)
		check_cxx_compiler_flag("${flag}" flag_supported)
		if(flag_supported)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
		endif()
		unset(flag_supported CACHE)
	endif()
endfunction()

# Collect implementation files under src/.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Enable extra warnings
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")

# Add binary
add_executable(${PROJECT_NAME})

# Add source files
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::JSONC ftxui::ftxui)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	RPG_ENEMY_DATABASE_PATH="${CMAKE_CURRENT_BINARY_DIR}/data/enemies.json"
	RPG_PLAYER_CLASS_DATABASE_PATH="${CMAKE_CURRENT_BINARY_DIR}/data/player_classes.json"
	RPG_SPELL_DATABASE_PATH="${CMAKE_CURRENT_BINARY_DIR}/data/spells.json"
	RPG_ITEM_DATABASE_PATH="${CMAKE_CURRENT_BINARY_DIR}/data/items.json"
	RPG_EQUIPMENT_DATABASE_PATH="${CMAKE_CURRENT_BINARY_DIR}/data/equipment.json")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")
configure_file(data/enemies.json "${CMAKE_CURRENT_BINARY_DIR}/data/enemies.json" COPYONLY)
configure_file(data/player_classes.json "${CMAKE_CURRENT_BINARY_DIR}/data/player_classes.json" COPYONLY)
configure_file(data/spells.json "${CMAKE_CURRENT_BINARY_DIR}/data/spells.json" COPYONLY)
configure_file(data/items.json "${CMAKE_CURRENT_BINARY_DIR}/data/items.json" COPYONLY)
configure_file(data/equipment.json "${CMAKE_CURRENT_BINARY_DIR}/data/equipment.json" COPYONLY)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE src)

include(CTest)
if(BUILD_TESTING)
	add_executable(enemy_generator_test
		tests/enemy_generator_test.cpp
		src/game/enemy/enemy_generator.cpp
		src/game/actions/fight/attack.cpp)
	target_include_directories(enemy_generator_test PRIVATE src)
	target_link_libraries(enemy_generator_test PRIVATE PkgConfig::JSONC)
	target_compile_definitions(enemy_generator_test PRIVATE
		RPG_ENEMY_DATABASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/enemies.json")
	add_test(NAME enemy_generator_test COMMAND enemy_generator_test)
	add_executable(player_class_generator_test
		tests/player_class_generator_test.cpp
		src/game/player/player_class_generator.cpp
		src/game/actions/fight/attack.cpp)
	target_include_directories(player_class_generator_test PRIVATE src)
	target_link_libraries(player_class_generator_test PRIVATE PkgConfig::JSONC)
	target_compile_definitions(player_class_generator_test PRIVATE
		RPG_PLAYER_CLASS_DATABASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/player_classes.json")
	add_test(NAME player_class_generator_test COMMAND player_class_generator_test)
	add_executable(leveling_test
		tests/leveling_test.cpp
		src/game/leveling.cpp
		src/game/actions/fight/attack.cpp)
	target_include_directories(leveling_test PRIVATE src)
	add_test(NAME leveling_test COMMAND leveling_test)
	add_executable(spell_database_test
		tests/spell_database_test.cpp
		src/game/actions/fight/spells.cpp
		src/game/actions/fight/attack.cpp)
	target_include_directories(spell_database_test PRIVATE src)
	target_link_libraries(spell_database_test PRIVATE PkgConfig::JSONC)
	target_compile_definitions(spell_database_test PRIVATE
		RPG_SPELL_DATABASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/spells.json")
	add_test(NAME spell_database_test COMMAND spell_database_test)
	add_executable(inventory_test
		tests/inventory_test.cpp
		src/game/player/player.cpp
		src/game/player/item_use_registry.cpp
		src/game/player/items/healing_potion.cpp
		src/game/player/items/mana_potion.cpp
		src/game/player/items/fire_scroll.cpp
		src/game/player/items/lightning_scroll.cpp
		src/game/player/items/amber_relic.cpp
		src/game/actions/fight/attack.cpp)
	target_include_directories(inventory_test PRIVATE src)
	add_test(NAME inventory_test COMMAND inventory_test)
	add_executable(item_database_test tests/item_database_test.cpp
		src/game/player/item_database.cpp)
	target_include_directories(item_database_test PRIVATE src)
	target_link_libraries(item_database_test PRIVATE PkgConfig::JSONC)
	target_compile_definitions(item_database_test PRIVATE
		RPG_ITEM_DATABASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/items.json")
	# Item catalog test also validates equipment definitions in their own file.
	target_compile_definitions(item_database_test PRIVATE
		RPG_EQUIPMENT_DATABASE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/equipment.json")
	add_test(NAME item_database_test COMMAND item_database_test)
endif()
