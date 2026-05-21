include(configuration/cmake/platform/linux.cmake)
include(configuration/cmake/platform/macos.cmake)
include(configuration/cmake/platform/windows.cmake)

# distribution.cmake owns the shared packaging entrypoint
# Platform modules append commands; bundle_resources remains the one target
function(configure_distribution targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    # Platform modules append extra release artifact work here
    # macOS uses a .app bundle; Linux uses an AppDir
    set(appBundleCommands)
    set(appBundleDependencies)

    # resources/ is the shared runtime data tree
    # Copies beside the raw executable and into platform bundles
    file(GLOB_RECURSE resourceFiles CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/resources/*
    )

    # Scene XML lives beside scene source code
    # Mirrors only XML into the runtime path expected by SceneLoader
    file(GLOB_RECURSE sceneResourceFiles CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/source/scenes/*.xml
    )

    set(sceneResourceCommands)

    # Raw executable builds still get source/scenes beside them
    # Keeps development and non-bundle artifacts runnable
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

    # Platform helpers own **OS-specific** packaging details
    # Keeps this file focused on shared orchestration
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

    # bundle_resources is the release packaging target
    # Verifies executable, runtime data, scene XML, and platform assets together
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

    # install exposes the raw executable/resources
    # Richer release artifacts are produced by bundle_resources
    install(TARGETS ${targetName}
        RUNTIME DESTINATION .
    )

    install(DIRECTORY resources
        DESTINATION .
    )
endfunction()
