# Tests are opt-in so release and packaging builds avoid test-only dependencies
option(BAPHOMETS_QUEST_BUILD_TESTS
    "Build unit tests"
    OFF
)

if(BAPHOMETS_QUEST_BUILD_TESTS)
    include(CTest)
endif()

# configure_unit_tests wires Catch2 tests into CTest
# Test cases are discovered individually so CTest reports useful granularity
function(configure_unit_tests engineTargetName)
    if(NOT BAPHOMETS_QUEST_BUILD_TESTS)
        return()
    endif()

    if(NOT TARGET ${engineTargetName})
        message(FATAL_ERROR "Target '${engineTargetName}' does not exist")
    endif()

    find_package(Catch2 3 REQUIRED)
    include(Catch)

    file(GLOB_RECURSE unitTestFiles CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/test/unit/*.cpp
    )

    add_executable(unit_tests
        ${unitTestFiles}
    )

    configure_compiler_options(unit_tests)
    target_link_libraries(unit_tests
        PRIVATE
            Catch2::Catch2WithMain
            ${engineTargetName}
    )

    catch_discover_tests(unit_tests)
endfunction()
