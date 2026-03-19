cmake_minimum_required(VERSION 3.20)
project(UniLogicProjectManager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ── Find Qt6 ─────────────────────────────────────────────────────────────
# Set Qt6_DIR or CMAKE_PREFIX_PATH to your Qt6 installation, e.g.:
#   cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2019_64" ..
#
# For a static build (no runtime dependencies, single .exe):
#   1. Build Qt6 from source with -static -static-runtime
#      OR use vcpkg: vcpkg install qt6:x64-windows-static
#   2. Point CMAKE_PREFIX_PATH to the static Qt prefix
#   3. The target will link statically automatically when Qt is static

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# ── Sources ──────────────────────────────────────────────────────────────
set(SOURCES
    main.cpp
    mainwindow.cpp
    settingsdialog.cpp
    welcomescreen.cpp
    filetreepanel.cpp
    editortab.cpp
    outputpanel.cpp
    repltab.cpp
    newprojecttab.cpp
    drconfiguratortab.cpp
    buildtab.cpp
    drconflicttab.cpp
    packagestab.cpp
    drdiffviewer.cpp
    normdiffviewer.cpp
    projectstats.cpp
    commandpalette.cpp
    templatechooser.cpp
    pluginmanager.cpp
    drmonitor.cpp
    exportdialog.cpp
    updatechecker.cpp
    setupwizard.cpp
    helpdialogs.cpp
)

set(HEADERS
    theme.h
    mainwindow.h
    settingsdialog.h
    welcomescreen.h
    filetreepanel.h
    editortab.h
    outputpanel.h
    repltab.h
    newprojecttab.h
    drconfiguratortab.h
    buildtab.h
    drconflicttab.h
    packagestab.h
    drdiffviewer.h
    normdiffviewer.h
    projectstats.h
    commandpalette.h
    templatechooser.h
    plugin.h
    pluginmanager.h
    drmonitor.h
    exportdialog.h
    updatechecker.h
    setupwizard.h
    helpdialogs.h
)

# ── Icon resource (Windows) ──────────────────────────────────────────────
# unilogic.ico lives in this directory (no spaces) so windres is happy.
if(WIN32)
    set(APP_ICON_RC "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
    if(EXISTS "${APP_ICON_RC}")
        list(APPEND SOURCES "${APP_ICON_RC}")
    endif()
endif()

# ── Executable ───────────────────────────────────────────────────────────
# WIN32 flag: creates a GUI app (no console window)
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)

# ── Windows system libraries ────────────────────────────────────────────
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE shell32 ole32)
endif()

# ── Static linking (MSVC) ───────────────────────────────────────────────
if(MSVC)
    # Use static CRT to eliminate MSVC runtime dependency
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ── Strip release builds (MinGW/GCC) ────────────────────────────────────
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Release>:-s>"
        "-static-libgcc" "-static-libstdc++"
    )
endif()
