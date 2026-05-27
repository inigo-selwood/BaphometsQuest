# Tests are opt-in so release and packaging builds avoid test-only dependencies
option(BAPHOMETS_QUEST_BUILD_TESTS
    "Build unit tests"
    OFF
)

# configure_unit_tests wires Catch2 tests into CTest
# Test files stay under test/unit and link the reusable engine target
function(configure_unit_tests engineTargetName)
    if(NOT BAPHOMETS_QUEST_BUILD_TESTS)
        return()
    endif()

    if(NOT TARGET ${engineTargetName})
        message(FATAL_ERROR "Target '${engineTargetName}' does not exist")
    endif()

    enable_testing()
    find_package(Catch2 3 REQUIRED)

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

    add_test(NAME unit_tests COMMAND unit_tests)
endfunction()
