cmake_minimum_required(VERSION 3.24)

project(UntitledGame
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(spdlog REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(tinyxml2 REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    source/*.cpp
    source/*.hpp
)

add_executable(untitled_game ${SOURCE_FILES})

target_link_libraries(untitled_game
    PRIVATE
        spdlog::spdlog
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
        tinyxml2::tinyxml2
)
