function(configure_linux_distribution
    targetName
    sceneResourceFilesVariable
    appBundleCommandsOutput
    appBundleDependenciesOutput
)
    # AppDir is the portable Linux application shape
    # Runs directly now; can feed AppImage tooling later
    set(linuxAppDirectory
        $<TARGET_FILE_DIR:${targetName}>/BaphometsQuest.AppDir
    )
    set(linuxAppRun
        ${CMAKE_BINARY_DIR}/generated/AppRun
    )
    set(linuxDesktopFile
        ${CMAKE_BINARY_DIR}/generated/baphomets_quest.desktop
    )

    # AppRun is the portable launch shim
    # Starts the real executable from usr/bin; resources stay beside the binary
    set(BAPHOMETS_QUEST_LINUX_EXECUTABLE ${targetName})
    configure_file(
        ${CMAKE_SOURCE_DIR}/configuration/distribution/linux/AppRun.in
        ${linuxAppRun}
        @ONLY
    )
    configure_file(
        ${CMAKE_SOURCE_DIR}/configuration/distribution/linux/baphomets_quest.desktop.in
        ${linuxDesktopFile}
        @ONLY
    )
    # AppRun must be executable inside the AppDir
    # CMake writes configured files with regular permissions by default
    file(CHMOD ${linuxAppRun}
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )

    # AppDir keeps top-level and freedesktop launcher metadata
    # Runs directly and stays friendly to tools expecting usr/share layout
    list(APPEND appBundleDependencies ${linuxAppRun} ${linuxDesktopFile})
    list(APPEND appBundleCommands
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${linuxAppDirectory}
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${linuxAppDirectory}/usr/bin
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${linuxAppDirectory}/usr/share/applications
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${linuxAppDirectory}/usr/share/icons/hicolor/32x32/apps
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${targetName}>
            ${linuxAppDirectory}/usr/bin/$<TARGET_FILE_NAME:${targetName}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${linuxAppRun}
            ${linuxAppDirectory}/AppRun
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${linuxDesktopFile}
            ${linuxAppDirectory}/baphomets_quest.desktop
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${linuxDesktopFile}
            ${linuxAppDirectory}/usr/share/applications/baphomets_quest.desktop
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/resources/textures/Favicon.png
            ${linuxAppDirectory}/baphomets_quest.png
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/resources/textures/Favicon.png
            ${linuxAppDirectory}/usr/share/icons/hicolor/32x32/apps/baphomets_quest.png
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            ${linuxAppDirectory}/usr/bin/resources
    )

    # Scene XML mirrors source/scenes under usr/bin
    # Runtime paths stay identical between local and AppDir runs
    foreach(sceneResourceFile ${${sceneResourceFilesVariable}})
        file(RELATIVE_PATH
            sceneResourcePath
            ${CMAKE_SOURCE_DIR}/source/scenes
            ${sceneResourceFile}
        )
        get_filename_component(sceneResourceDirectory
            ${sceneResourcePath}
            DIRECTORY
        )

        list(APPEND appBundleCommands
            COMMAND ${CMAKE_COMMAND} -E make_directory
                ${linuxAppDirectory}/usr/bin/source/scenes/${sceneResourceDirectory}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${sceneResourceFile}
                ${linuxAppDirectory}/usr/bin/source/scenes/${sceneResourcePath}
        )
    endforeach()

    set(${appBundleCommandsOutput} ${appBundleCommands} PARENT_SCOPE)
    set(${appBundleDependenciesOutput} ${appBundleDependencies} PARENT_SCOPE)
endfunction()
