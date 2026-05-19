include(CheckIPOSupported)

option(BAPHOMETS_QUEST_RELEASE_OPTIMISATION
    "Enable compiler optimisations for Release builds"
    ON
)
option(BAPHOMETS_QUEST_NATIVE_RELEASE_ARCH
    "Optimise Release builds for the current machine architecture"
    ON
)

function(configure_compiler_options targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    target_compile_features(${targetName}
        PRIVATE
            cxx_std_20
    )

    set_target_properties(${targetName}
        PROPERTIES
            CXX_EXTENSIONS OFF
    )

    if(BAPHOMETS_QUEST_RELEASE_OPTIMISATION)
        check_ipo_supported(RESULT ipoSupported OUTPUT ipoError)

        if(ipoSupported)
            set_property(TARGET ${targetName}
                PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
            )
        else()
            message(STATUS "Release IPO/LTO unavailable: ${ipoError}")
        endif()

        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${targetName}
                PRIVATE
                    $<$<CONFIG:Release>:-O3>
                    $<$<CONFIG:Release>:-ffast-math>
                    $<$<CONFIG:Release>:-fomit-frame-pointer>
            )

            if(BAPHOMETS_QUEST_NATIVE_RELEASE_ARCH)
                target_compile_options(${targetName}
                    PRIVATE
                        $<$<CONFIG:Release>:-march=native>
                )
            endif()
        elseif(MSVC)
            target_compile_options(${targetName}
                PRIVATE
                    $<$<CONFIG:Release>:/O2>
                    $<$<CONFIG:Release>:/Ob3>
                    $<$<CONFIG:Release>:/fp:fast>
            )
        endif()
    endif()
endfunction()
