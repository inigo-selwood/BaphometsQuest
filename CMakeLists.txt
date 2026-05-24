cmake_minimum_required(VERSION 3.24)

project(BaphometsQuest
    VERSION 0.1.0
    LANGUAGES CXX
)

option(BAPHOMETS_QUEST_BUILD_TESTS
    "Build unit tests"
    OFF
)

include(configuration/cmake/compiler_options.cmake)
include(configuration/cmake/dependencies.cmake)
include(configuration/cmake/distribution.cmake)
include(configuration/cmake/platform_resize_handler.cmake)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    source/*.cpp
)

exclude_resize_handler_platform_sources(SOURCE_FILES)

set(ENGINE_SOURCE_FILES ${SOURCE_FILES})
list(REMOVE_ITEM ENGINE_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp
)

add_library(baphomets_quest_engine STATIC
    ${ENGINE_SOURCE_FILES}
)

target_include_directories(baphomets_quest_engine
    PUBLIC
        source
)

configure_compiler_options(baphomets_quest_engine)
configure_dependencies(baphomets_quest_engine)
configure_resize_handler(baphomets_quest_engine)

add_executable(baphomets_quest
    source/main.cpp
)

configure_compiler_options(baphomets_quest)
target_link_libraries(baphomets_quest
    PRIVATE
        baphomets_quest_engine
)
configure_distribution(baphomets_quest)

if(BAPHOMETS_QUEST_BUILD_TESTS)
    enable_testing()
    find_package(Catch2 3 REQUIRED)

    file(GLOB_RECURSE UNIT_TEST_FILES CONFIGURE_DEPENDS
        test/unit/*.cpp
    )

    add_executable(unit_tests
        ${UNIT_TEST_FILES}
    )

    configure_compiler_options(unit_tests)
    target_link_libraries(unit_tests
        PRIVATE
            Catch2::Catch2WithMain
            baphomets_quest_engine
    )

    add_test(NAME unit_tests COMMAND unit_tests)
endif()
