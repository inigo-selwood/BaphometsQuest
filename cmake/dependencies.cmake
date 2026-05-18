function(configure_dependencies targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    find_package(spdlog REQUIRED)
    find_package(CLI11 REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(tinyxml2 REQUIRED)
    find_package(yaml-cpp REQUIRED)

    target_link_libraries(${targetName}
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
endfunction()
