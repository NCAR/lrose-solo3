# cmake for solo3

cmake_minimum_required(VERSION 2.8)

# project name and build type

project(solo3)
set(CMAKE_BUILD_TYPE Release)

# Where to find *Find*.cmake and *-config.cmake

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_PREFIX_PATH ${CMAKE_MODULE_PATH})

# Use the package PkgConfig to detect GTK+ headers/library files

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtkmm-3.0)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# enable testing for this dir and below

enable_testing()

# Set RPATH so that executablefinds lrose libraries without needing LD_LIBRARY_PATH

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set(CMAKE_INSTALL_RPATH ${LROSE_PREFIX}/lib)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Use RUNPATH, not RPATH

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
endif()

# recurse into src directory for the build

add_subdirectory(translate) 
add_subdirectory(editor) 
add_subdirectory(perusal) 

