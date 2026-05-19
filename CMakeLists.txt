cmake_minimum_required(VERSION 3.24)

project(BaphometsQuest
    VERSION 0.1.0
    LANGUAGES CXX
)

include(configuration/cmake/compiler_options.cmake)
include(configuration/cmake/dependencies.cmake)
include(configuration/cmake/distribution.cmake)
include(configuration/cmake/platform_resize_handler.cmake)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    source/*.cpp
)

exclude_resize_handler_platform_sources(SOURCE_FILES)

add_executable(baphomets_quest
    ${SOURCE_FILES}
)

configure_compiler_options(baphomets_quest)
configure_dependencies(baphomets_quest)
configure_resize_handler(baphomets_quest)
configure_distribution(baphomets_quest)
