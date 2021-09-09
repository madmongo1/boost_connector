
file(GLOB_RECURSE spot_book_src CMAKE_CONFIGURE_DEPENDS "*.cpp" "*.hpp")
add_executable(boost_connector-example-spot_book ${spot_book_src})
target_link_libraries(boost_connector-example-spot_book PUBLIC Boost::connector)
