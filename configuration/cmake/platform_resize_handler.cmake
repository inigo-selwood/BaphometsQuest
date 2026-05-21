# Native aspect locking is best-effort
# Fallback handler keeps resize behaviour portable when APIs are missing
option(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK
    "Use native platform APIs to constrain window aspect ratio during resize"
    ON
)

# Resize sources are globbed at the root
# Removes all platform handlers first; configure_resize_handler adds one back
function(exclude_resize_handler_platform_sources sourceFiles)
    set(platformSources
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/fallback.cpp
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/platform/macos.cpp
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/platform/windows.cpp
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/platform/x11.cpp
    )

    list(REMOVE_ITEM ${sourceFiles} ${platformSources})
    set(${sourceFiles} ${${sourceFiles}} PARENT_SCOPE)
endfunction()

# configure_resize_handler selects the platform resize implementation
# Exposes matching compile definitions for runtime reporting
function(configure_resize_handler targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    set(platformName "unknown")
    set(handlerName "fallback")
    set(platformSource
        ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/fallback.cpp
    )
    set(usesMacos OFF)
    set(usesWindows OFF)
    set(usesX11 OFF)

    # macOS native aspect locking uses Cocoa
    # Only compiled when native aspect locking is enabled
    if(APPLE)
        set(platformName "macos")

        if(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK)
            set(handlerName "macos")
            set(platformSource
                ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/platform/macos.cpp
            )
            set(usesMacos ON)
        endif()
    # Windows native aspect locking uses user32
    # Only linked when the native handler is selected
    elseif(WIN32)
        set(platformName "windows")

        if(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK)
            set(handlerName "windows")
            set(platformSource
                ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/platform/windows.cpp
            )
            set(usesWindows ON)
        endif()
    # Linux native aspect locking currently means X11
    # Wayland and other sessions fall back until a handler exists
    elseif(UNIX)
        set(platformName "linux")

        if(BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK)
            find_package(X11 QUIET)

            if(X11_FOUND)
                set(handlerName "x11")
                set(platformSource
                    ${CMAKE_SOURCE_DIR}/source/engine/runtime/resize/platform/x11.cpp
                )
                set(usesX11 ON)
            endif()
        endif()
    endif()

    message(STATUS "Window resize handler: ${handlerName}")

    # Only the **selected implementation** is added after glob filtering
    # Avoids compiling platform files with unavailable OS headers
    target_sources(${targetName}
        PRIVATE
            ${platformSource}
    )

    # Platform definitions are present in every build
    # Runtime code can branch without repeating CMake platform logic
    target_compile_definitions(${targetName}
        PRIVATE
            BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK=$<BOOL:${BAPHOMETS_QUEST_NATIVE_ASPECT_LOCK}>
            BAPHOMETS_QUEST_PLATFORM_NAME=\"${platformName}\"
    )

    if(APPLE)
        # Cocoa support needs Objective-C runtime linkage
        # The framework alone is not enough for the native resize implementation
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
        # Windows headers are kept lean
        # Prevents min/max macros colliding with standard-library names
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
        # Linux is broader than X11
        # X11 gets its own definition only when that backend is compiled
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
