include(configuration/cmake/platform/linux.cmake)
include(configuration/cmake/platform/macos.cmake)
include(configuration/cmake/platform/windows.cmake)

function(configure_distribution targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    set(appBundleCommands)
    set(appBundleDependencies)

    file(GLOB_RECURSE resourceFiles CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/resources/*
    )

    file(GLOB_RECURSE sceneResourceFiles CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/source/scenes/*.xml
    )

    set(sceneResourceCommands)

    foreach(sceneResourceFile ${sceneResourceFiles})
        file(RELATIVE_PATH
            sceneResourcePath
            ${CMAKE_SOURCE_DIR}/source/scenes
            ${sceneResourceFile}
        )
        get_filename_component(sceneResourceDirectory
            ${sceneResourcePath}
            DIRECTORY
        )

        list(APPEND sceneResourceCommands
            COMMAND ${CMAKE_COMMAND} -E make_directory
                $<TARGET_FILE_DIR:${targetName}>/source/scenes/${sceneResourceDirectory}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${sceneResourceFile}
                $<TARGET_FILE_DIR:${targetName}>/source/scenes/${sceneResourcePath}
        )
    endforeach()

    if(WIN32)
        configure_windows_distribution(${targetName})
    elseif(APPLE)
        configure_macos_distribution(
            ${targetName}
            sceneResourceFiles
            appBundleCommands
            appBundleDependencies
        )
    elseif(UNIX)
        configure_linux_distribution(
            ${targetName}
            sceneResourceFiles
            appBundleCommands
            appBundleDependencies
        )
    endif()

    add_custom_target(bundle_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${targetName}>/resources
        ${sceneResourceCommands}
        ${appBundleCommands}
        DEPENDS ${resourceFiles}
            ${sceneResourceFiles}
            ${appBundleDependencies}
        COMMENT "Bundling resources"
    )

    add_dependencies(bundle_resources ${targetName})

    install(TARGETS ${targetName}
        RUNTIME DESTINATION .
    )

    install(DIRECTORY resources
        DESTINATION .
    )
endfunction()
