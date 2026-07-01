cmake_minimum_required(VERSION 3.15)
project(oauth2loopback CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpprestsdk CONFIG REQUIRED)

add_library(oauth2loopback
	src/authorization_flow.cpp
	src/code_listener.cpp
)
target_include_directories(oauth2loopback PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(oauth2loopback PUBLIC cpprestsdk::cpprest)

# cpprestsdk uses OpenSSL on non-Windows platforms but some packagings
# (e.g. Homebrew) do not propagate it through their CMake config.
if(NOT WIN32)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(oauth2loopback PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()

option(OAUTH2LOOPBACK_BUILD_EXAMPLES "Build example programs" ON)
if(OAUTH2LOOPBACK_BUILD_EXAMPLES)
	add_executable(dropbox_example examples/dropbox_example.cpp)
	target_link_libraries(dropbox_example PRIVATE oauth2loopback)
endif()
