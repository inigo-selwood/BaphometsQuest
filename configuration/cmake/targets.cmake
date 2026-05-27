# collect_engine_sources keeps source discovery in one place
# The app executable is excluded so tests can link the engine library directly
function(collect_engine_sources outputVariable)
    file(GLOB_RECURSE engineSourceFiles CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/source/*.cpp
    )

    exclude_resize_handler_platform_sources(engineSourceFiles)

    list(REMOVE_ITEM engineSourceFiles
        ${CMAKE_SOURCE_DIR}/source/main.cpp
    )

    set(${outputVariable} ${engineSourceFiles} PARENT_SCOPE)
endfunction()

# configure_engine_target builds the reusable engine library
# Runtime app and tests both link this target
function(configure_engine_target targetName)
    collect_engine_sources(engineSourceFiles)

    add_library(${targetName} STATIC
        ${engineSourceFiles}
    )

    target_include_directories(${targetName}
        PUBLIC
            ${CMAKE_SOURCE_DIR}/source
    )

    configure_compiler_options(${targetName})
    configure_dependencies(${targetName})
    configure_resize_handler(${targetName})
endfunction()

# configure_game_target builds the shipped executable
# Platform packaging is applied only to this app target
function(configure_game_target targetName engineTargetName)
    if(NOT TARGET ${engineTargetName})
        message(FATAL_ERROR "Target '${engineTargetName}' does not exist")
    endif()

    add_executable(${targetName}
        ${CMAKE_SOURCE_DIR}/source/main.cpp
    )

    configure_compiler_options(${targetName})
    target_link_libraries(${targetName}
        PRIVATE
            ${engineTargetName}
    )
    configure_distribution(${targetName})
endfunction()
