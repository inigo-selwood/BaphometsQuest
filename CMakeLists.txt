cmake_minimum_required(VERSION 3.24)

project(UntitledGame
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

add_executable(untitled_game ${SOURCE_FILES})

target_link_libraries(untitled_game
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

add_custom_target(bundle_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:untitled_game>/resources
    DEPENDS ${RESOURCE_FILES}
    COMMENT "Bundling resources"
)

add_dependencies(bundle_resources untitled_game)

install(TARGETS untitled_game
    RUNTIME DESTINATION .
)

install(DIRECTORY resources
    DESTINATION .
)
