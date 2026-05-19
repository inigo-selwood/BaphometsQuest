function(configure_distribution targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

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

    add_custom_target(bundle_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${targetName}>/resources
        ${sceneResourceCommands}
        DEPENDS ${resourceFiles}
            ${sceneResourceFiles}
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
