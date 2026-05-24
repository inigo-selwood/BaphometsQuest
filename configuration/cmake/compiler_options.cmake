include(CheckIPOSupported)

# Release optimisation is an explicit project option
# CI, packaging, and local debugging can choose performance or predictability
option(BAPHOMETS_QUEST_RELEASE_OPTIMISATION
    "Enable compiler optimisations for Release builds"
    ON
)
# Native architecture flags are useful for local release builds
# CI disables them so artifacts are not tuned to one runner's CPU
option(BAPHOMETS_QUEST_NATIVE_RELEASE_ARCH
    "Optimise Release builds for the current machine architecture"
    ON
)
option(BAPHOMETS_QUEST_ENABLE_COVERAGE
    "Enable Clang source coverage flags for test builds"
    OFF
)

# configure_compiler_options owns compiler policy
# Keeps feature, optimisation, and compiler-family conditionals in one place
function(configure_compiler_options targetName)
    if(NOT TARGET ${targetName})
        message(FATAL_ERROR "Target '${targetName}' does not exist")
    endif()

    # The engine uses modern C++ features directly
    # Compiler extensions stay off so platform builds stay portable
    target_compile_features(${targetName}
        PRIVATE
            cxx_std_20
    )

    set_target_properties(${targetName}
        PROPERTIES
            CXX_EXTENSIONS OFF
    )

    if(BAPHOMETS_QUEST_RELEASE_OPTIMISATION)
        # IPO/LTO is valuable when available
        # CMake checks support per toolchain so unsupported compilers degrade
        check_ipo_supported(RESULT ipoSupported OUTPUT ipoError)

        if(ipoSupported)
            set_property(TARGET ${targetName}
                PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
            )
        else()
            message(STATUS "Release IPO/LTO unavailable: ${ipoError}")
        endif()

        # GCC and Clang share performance flag spelling
        # Native CPU tuning is gated separately so scheduled CI can opt out
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
        # MSVC has separate flag spelling
        # Optimises release builds without changing debug builds
        elseif(MSVC)
            target_compile_options(${targetName}
                PRIVATE
                    $<$<CONFIG:Release>:/O2>
                    $<$<CONFIG:Release>:/Ob3>
                    $<$<CONFIG:Release>:/fp:fast>
            )
        endif()
    endif()

    if(BAPHOMETS_QUEST_ENABLE_COVERAGE)
        if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            message(FATAL_ERROR "Coverage is currently configured for Clang")
        endif()

        target_compile_options(${targetName}
            PRIVATE
                -fprofile-instr-generate
                -fcoverage-mapping
        )
        target_link_options(${targetName}
            PRIVATE
                -fprofile-instr-generate
                -fcoverage-mapping
        )
    endif()
endfunction()
