function(configure_distribution targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    file(GLOB_RECURSE resourceFiles CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/resources/*
    )

    add_custom_target(bundle_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${targetName}>/resources
        DEPENDS ${resourceFiles}
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
