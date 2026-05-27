cmake_minimum_required(VERSION 3.24)

project(BaphometsQuest
    VERSION 0.1.0
    LANGUAGES CXX
)

include(configuration/cmake/compiler_options.cmake)
include(configuration/cmake/dependencies.cmake)
include(configuration/cmake/distribution.cmake)
include(configuration/cmake/platform_resize_handler.cmake)
include(configuration/cmake/targets.cmake)
include(configuration/cmake/tests.cmake)

configure_engine_target(baphomets_quest_engine)
configure_game_target(baphomets_quest baphomets_quest_engine)
configure_unit_tests(baphomets_quest_engine)
