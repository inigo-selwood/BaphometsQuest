# Dependencies are resolved through package config/CMake packages
# Local and CI builds stay honest about required installed libraries
function(configure_dependencies targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    # Package discovery happens before linking
    # Missing platform packages fail early with a clear CMake error
    find_package(spdlog REQUIRED)
    find_package(CLI11 REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(tinyxml2 REQUIRED)
    find_package(yaml-cpp REQUIRED)

    # Imported targets carry include paths and linker details
    # Keeps package-specific flags out of project target code
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
