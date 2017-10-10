# Minimum supported CMake version
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Project
project(asio
  VERSION 1.11.0
  LANGUAGES CXX)

# Options
set(asio_BUILD_EXAMPLES OFF CACHE BOOL "Build asio examples")

# Installation configuration
include(GNUInstallDirs)
if(NOT DEFINED CMAKE_INSTALL_CMAKEDIR)
  set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/asio)
endif()

# Interface library
add_subdirectory(./include)

# Sources
add_subdirectory(./src)

# Installation
configure_file(asio-config-version.cmake.in asio-config-version.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/asio-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
install(EXPORT asio-targets
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
  NAMESPACE asio::)
install(FILES asio-config.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
