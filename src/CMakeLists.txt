cmake_minimum_required(VERSION 3.15)

project(keypresenter)

set(KEYPRESENTER_INCLUDES ../include)

# PkgConfig
find_package(PkgConfig REQUIRED)
if (NOT PKGCONFIG_FOUND)
    message(FATAL_ERROR "PkgConfig not found!")
endif()

# gtkmm
pkg_check_modules(GTK REQUIRED gtk+-3.0)
list(APPEND KEYPRESENTER_INCLUDES ${GTK_INCLUDE_DIRS})
list(APPEND KEYPRESENTER_DEPENDENCIES ${GTK_LIBRARIES})

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    list(APPEND KEYPRESENTER_DEPENDENCIES ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
    list(APPEND KEYPRESENTER_KEYBOARD_IMPL ${IOKIT_LIBRARY})
elseif(UNIX)
    find_package(Threads REQUIRED)
    find_library(x11 NAMES X11)
    find_library(xrandr NAMES Xrandr)
    find_library(xcursor NAMES Xcursor)
    find_library(xi NAMES Xi)
    find_library(xxf86vm NAMES Xxf86vm)
    find_library(dl NAMES dl)
    find_library(xinerama NAMES Xinerama)
    list(APPEND KEYPRESENTER_DEPENDENCIES Threads::Threads ${x11} ${xrandr} ${xi} ${xxf86vm} ${dl} ${xcursor} ${xinerama})
    add_compile_definitions(KEYPRESENTER_BUILD_USE_X11)
    list(APPEND KEYPRESENTER_KEYBOARD_IMPL x11.c)
    list(APPEND KEYPRESENTER_KEYBOARD_IMPL x11.h)
endif()


add_executable(keypresenter
                    appstate.h
                    macro.h
                    main.c
                    polltaskresult.h
                    ${KEYPRESENTER_KEYBOARD_IMPL}
                    ../include/keypresenter/keypresenter.h)

target_include_directories(keypresenter PRIVATE ${KEYPRESENTER_INCLUDES})
target_link_libraries(keypresenter ${KEYPRESENTER_DEPENDENCIES})

install(
        TARGETS keypresenter
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)