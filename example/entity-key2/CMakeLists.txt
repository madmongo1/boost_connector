
file(GLOB_RECURSE entity-key2_src CMAKE_CONFIGURE_DEPENDS "*.cpp" "*.hpp")
add_executable(boost_connector-example-entity-key2 ${entity-key2_src})
target_link_libraries(boost_connector-example-entity-key2 PUBLIC Boost::connector Boost::json)
