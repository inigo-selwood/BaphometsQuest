function(configure_macos_distribution
    targetName
    sceneResourceFilesVariable
    appBundleCommandsOutput
    appBundleDependenciesOutput
)
    set(appBundleName BaphometsQuest.app)
    set(appBundleDirectory
        $<TARGET_FILE_DIR:${targetName}>/${appBundleName}
    )
    set(appIconFile
        ${CMAKE_BINARY_DIR}/generated/baphomets_quest.icns
    )
    set(appIconSource
        ${CMAKE_SOURCE_DIR}/resources/textures/Favicon.png
    )
    set(appInfoPlist
        ${CMAKE_BINARY_DIR}/generated/Info.plist
    )

    set(BAPHOMETS_QUEST_BUNDLE_EXECUTABLE ${targetName})
    set(BAPHOMETS_QUEST_BUNDLE_ICON baphomets_quest.icns)
    set(BAPHOMETS_QUEST_BUNDLE_IDENTIFIER
        com.inigo-selwood.baphomets-quest
    )
    set(BAPHOMETS_QUEST_BUNDLE_VERSION ${PROJECT_VERSION})
    configure_file(
        ${CMAKE_SOURCE_DIR}/configuration/distribution/macos/Info.plist.in
        ${appInfoPlist}
        @ONLY
    )

    add_custom_command(
        OUTPUT ${appIconFile}
        COMMAND /usr/bin/sips -s format icns ${appIconSource} --out
            ${appIconFile}
        DEPENDS ${appIconSource}
        COMMENT "Generating macOS app icon"
    )

    list(APPEND appBundleDependencies ${appIconFile} ${appInfoPlist})
    list(APPEND appBundleCommands
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${appBundleDirectory}
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${appBundleDirectory}/Contents/MacOS
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${appBundleDirectory}/Contents/Resources
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${targetName}>
            ${appBundleDirectory}/Contents/MacOS/$<TARGET_FILE_NAME:${targetName}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${appInfoPlist}
            ${appBundleDirectory}/Contents/Info.plist
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${appIconFile}
            ${appBundleDirectory}/Contents/Resources/baphomets_quest.icns
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            ${appBundleDirectory}/Contents/MacOS/resources
    )

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
                ${appBundleDirectory}/Contents/MacOS/source/scenes/${sceneResourceDirectory}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${sceneResourceFile}
                ${appBundleDirectory}/Contents/MacOS/source/scenes/${sceneResourcePath}
        )
    endforeach()

    set(${appBundleCommandsOutput} ${appBundleCommands} PARENT_SCOPE)
    set(${appBundleDependenciesOutput} ${appBundleDependencies} PARENT_SCOPE)
endfunction()
