function(configure_windows_distribution targetName)
    # Windows executable icons are compiled resources
    # RC lets CMake feed the generated .rc file into the resource compiler
    enable_language(RC)

    # Favicon.png is the single icon source
    # ImageMagick creates the multi-size .ico Windows expects
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
    # application.rc.in points at the generated .ico
    # The actual icon output belongs in the build directory
    configure_file(
        ${CMAKE_SOURCE_DIR}/configuration/distribution/windows/application.rc.in
        ${CMAKE_BINARY_DIR}/generated/application.rc
        @ONLY
    )
    # The executable depends on the generated icon
    # Prevents clean builds racing the .rc compiler input
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
