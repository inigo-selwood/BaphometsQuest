option(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK
    "Use native platform APIs to constrain window aspect ratio during resize"
    ON
)

function(exclude_resize_handler_platform_sources sourceFiles)
    set(platformSources
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_fallback.cpp
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_macos.cpp
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_windows.cpp
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_x11.cpp
    )

    list(REMOVE_ITEM ${sourceFiles} ${platformSources})
    set(${sourceFiles} ${${sourceFiles}} PARENT_SCOPE)
endfunction()

function(configure_resize_handler targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    set(platformName "unknown")
    set(handlerName "fallback")
    set(platformSource
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_fallback.cpp
    )
    set(usesMacos OFF)
    set(usesWindows OFF)
    set(usesX11 OFF)

    if(APPLE)
        set(platformName "macos")

        if(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK)
            set(handlerName "macos")
            set(platformSource
                ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_macos.cpp
            )
            set(usesMacos ON)
        endif()
    elseif(WIN32)
        set(platformName "windows")

        if(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK)
            set(handlerName "windows")
            set(platformSource
                ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_windows.cpp
            )
            set(usesWindows ON)
        endif()
    elseif(UNIX)
        set(platformName "linux")

        if(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK)
            find_package(X11 QUIET)

            if(X11_FOUND)
                set(handlerName "x11")
                set(platformSource
                    ${CMAKE_SOURCE_DIR}/source/engine/runtime/platform/resize_handler_x11.cpp
                )
                set(usesX11 ON)
            endif()
        endif()
    endif()

    message(STATUS "Window resize handler: ${handlerName}")

    target_sources(${targetName}
        PRIVATE
            ${platformSource}
    )

    target_compile_definitions(${targetName}
        PRIVATE
            BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK=$<BOOL:${BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK}>
            BAPHOMETS_QUEST_PLATFORM_NAME=\"${platformName}\"
    )

    if(APPLE)
        target_compile_definitions(${targetName}
            PRIVATE
                BAPHOMETS_QUEST_PLATFORM_MACOS=1
        )

        if(usesMacos)
            target_link_libraries(${targetName}
                PRIVATE
                    "-framework Cocoa"
                    objc
            )
        endif()
    elseif(WIN32)
        target_compile_definitions(${targetName}
            PRIVATE
                BAPHOMETS_QUEST_PLATFORM_WINDOWS=1
                NOMINMAX
                WIN32_LEAN_AND_MEAN
        )

        if(usesWindows)
            target_link_libraries(${targetName}
                PRIVATE
                    user32
            )
        endif()
    elseif(UNIX)
        target_compile_definitions(${targetName}
            PRIVATE
                BAPHOMETS_QUEST_PLATFORM_LINUX=1
        )

        if(usesX11)
            target_compile_definitions(${targetName}
                PRIVATE
                    BAPHOMETS_QUEST_PLATFORM_X11=1
            )

            target_include_directories(${targetName}
                PRIVATE
                    ${X11_INCLUDE_DIR}
            )

            target_link_libraries(${targetName}
                PRIVATE
                    ${X11_LIBRARIES}
            )
        endif()
    endif()
endfunction()
