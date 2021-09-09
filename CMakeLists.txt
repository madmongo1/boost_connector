cmake_minimum_required(VERSION 3.20)

project(boost_connector VERSION 0.1)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)


file(GLOB_RECURSE boost_connector_include CONFIGURE_DEPENDS "include/*.hpp")
file(GLOB_RECURSE boost_connector_src CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp")
add_library(boost_connector ${boost_connector_src} ${boost_connector_include})
target_link_libraries(boost_connector PUBLIC Boost::boost)
target_link_libraries(boost_connector PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(boost_connector PUBLIC Threads::Threads)
target_include_directories(boost_connector PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
set_property(TARGET boost_connector PROPERTY EXPORT_NAME "connector")
add_library(Boost::connector ALIAS boost_connector)

add_subdirectory(test)
add_subdirectory(example)
