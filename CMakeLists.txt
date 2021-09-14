cmake_minimum_required(VERSION 3.20)
include(cmake/library_props.cmake)
project(boost_connector VERSION 0.1)

if (NOT BUILD_SHARED_LIBS)
    set(Boost_USE_STATIC_LIBS ON)
endif()

option(BOOST_CONNECTOR_BUILD_TESTS ON)

find_package(Boost REQUIRED COMPONENTS json)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)


file(GLOB_RECURSE boost_connector_include CONFIGURE_DEPENDS "include/*.hpp")
file(GLOB_RECURSE boost_connector_src CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp")
set(boost_connector_spec ${boost_connector_src})
list(FILTER boost_connector_spec INCLUDE REGEX ".*\\.spec\\.[ch]pp")
list(FILTER boost_connector_src EXCLUDE REGEX ".*\\.spec\\.[ch]pp")

add_library(boost_connector ${boost_connector_src} ${boost_connector_include})
target_link_libraries(boost_connector PUBLIC Boost::boost Boost::json)
target_link_libraries(boost_connector PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(boost_connector PUBLIC Threads::Threads)
target_include_directories(boost_connector PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
set_property(TARGET boost_connector PROPERTY EXPORT_NAME "connector")
add_library(Boost::connector ALIAS boost_connector)

if (BOOST_CONNECTOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
add_subdirectory(example)
