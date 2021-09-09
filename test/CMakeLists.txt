file(GLOB_RECURSE test_src CONFIGURE_DEPENDS "*.cpp" "*.hpp")
add_executable(boost_connector_test ${test_src})
target_link_libraries(boost_connector_test PUBLIC Boost::connector)

add_test(boost_connector_test COMMAND boost_connector_test)
