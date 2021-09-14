include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.7
)
FetchContent_MakeAvailable(catch2)

file(GLOB_RECURSE test_src CONFIGURE_DEPENDS "*.cpp" "*.hpp")
add_executable(boost_connector_test ${boost_connector_spec} ${test_src})
target_link_libraries(boost_connector_test PUBLIC Boost::connector)
target_link_libraries(boost_connector_test PUBLIC Catch2::Catch2)

add_test(NAME boost_connector_test COMMAND boost_connector_test)
