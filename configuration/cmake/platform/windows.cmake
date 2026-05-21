function(configure_windows_distribution targetName)
    enable_language(RC)

    find_program(MAGICK_EXECUTABLE magick)

    if(NOT MAGICK_EXECUTABLE)
        message(FATAL_ERROR "Windows icon generation requires ImageMagick")
    endif()

    set(windowsIconSource
        ${CMAKE_SOURCE_DIR}/resources/textures/Favicon.png
    )
    set(windowsIconFile
        ${CMAKE_BINARY_DIR}/generated/baphomets_quest.ico
    )
    set(BAPHOMETS_QUEST_WINDOWS_ICON
        ${windowsIconFile}
    )
    configure_file(
        ${CMAKE_SOURCE_DIR}/configuration/distribution/windows/application.rc.in
        ${CMAKE_BINARY_DIR}/generated/application.rc
        @ONLY
    )
    add_custom_command(
        OUTPUT ${windowsIconFile}
        COMMAND ${MAGICK_EXECUTABLE}
            ${windowsIconSource}
            -define icon:auto-resize=256,128,64,48,32,16
            ${windowsIconFile}
        DEPENDS ${windowsIconSource}
        COMMENT "Generating Windows app icon"
    )
    add_custom_target(generate_windows_icon DEPENDS ${windowsIconFile})
    add_dependencies(${targetName} generate_windows_icon)
    target_sources(${targetName} PRIVATE
        ${CMAKE_BINARY_DIR}/generated/application.rc
    )
endfunction()
