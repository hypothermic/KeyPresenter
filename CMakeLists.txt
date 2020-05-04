cmake_minimum_required(VERSION 3.15)
project(keypresenter VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)


find_package(PkgConfig REQUIRED)
if (NOT PKGCONFIG_FOUND)
    message(FATAL_ERROR "PkgConfig not found!")
endif()

pkg_check_modules(GLIB REQUIRED gtk+-3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")


option(BUILD_INSTALLER "Build Installer" ON)
if(BUILD_INSTALLER)
    add_subdirectory(installer)
endif()

option(BUILD_UI "Build Keypresenter UI" ON)
if(BUILD_UI)
    add_subdirectory(src)
endif()