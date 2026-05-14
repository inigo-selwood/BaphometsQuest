cmake_minimum_required(VERSION 3.24)

project(BaphometsQuest
    VERSION 0.1.0
    LANGUAGES CXX
)

include(CheckIPOSupported)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BAPHOMETS_QUEST_RELEASE_OPTIMISATION
    "Enable compiler optimisations for Release builds"
    ON
)
option(BAPHOMETS_QUEST_NATIVE_RELEASE_ARCH
    "Optimise Release builds for the current machine architecture"
    ON
)

find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(yaml-cpp REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    source/*.cpp
    source/*.hpp
)

file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS
    resources/*
)

add_executable(baphomets_quest ${SOURCE_FILES})

if(BAPHOMETS_QUEST_RELEASE_OPTIMISATION)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

    if(IPO_SUPPORTED)
        set_property(TARGET baphomets_quest
            PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
        )
    else()
        message(STATUS "Release IPO/LTO unavailable: ${IPO_ERROR}")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(baphomets_quest
            PRIVATE
                $<$<CONFIG:Release>:-O3>
                $<$<CONFIG:Release>:-ffast-math>
                $<$<CONFIG:Release>:-fomit-frame-pointer>
        )

        if(BAPHOMETS_QUEST_NATIVE_RELEASE_ARCH)
            target_compile_options(baphomets_quest
                PRIVATE
                    $<$<CONFIG:Release>:-march=native>
            )
        endif()
    elseif(MSVC)
        target_compile_options(baphomets_quest
            PRIVATE
                $<$<CONFIG:Release>:/O2>
                $<$<CONFIG:Release>:/Ob3>
                $<$<CONFIG:Release>:/fp:fast>
        )
    endif()
endif()

target_link_libraries(baphomets_quest
    PRIVATE
        CLI11::CLI11
        spdlog::spdlog
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
        tinyxml2::tinyxml2
        yaml-cpp::yaml-cpp
)

add_custom_target(bundle_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:baphomets_quest>/resources
    DEPENDS ${RESOURCE_FILES}
    COMMENT "Bundling resources"
)

add_dependencies(bundle_resources baphomets_quest)

install(TARGETS baphomets_quest
    RUNTIME DESTINATION .
)

install(DIRECTORY resources
    DESTINATION .
)
