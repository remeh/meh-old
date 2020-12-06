# macOS build: CMAKE_PREFIX_PATH=/usr/local/Cellar/qt/5.14.1 cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../

cmake_minimum_required(VERSION 3.1.0)

project(meh)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCES
    main.cpp
    resources.qrc
    buffer.cpp
    command.cpp
    completer.cpp
    editor.cpp
    exec.cpp
    fileslookup.cpp
    git.cpp
    grep.cpp
    leader.cpp
    line_number_area.cpp
    lsp.cpp
    lsp_manager.cpp
    lsp/clangd.cpp
    lsp/gopls.cpp
    lsp/zls.cpp
    normal.cpp
    references_widget.cpp
    statusbar.cpp
    submode.cpp
    syntax.cpp
    tasks.cpp
    visual.cpp
    window.cpp
)

if (APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME meh)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "io.remy.meh")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    add_executable(meh MACOSX_BUNDLE ${SOURCES})
else()
    add_executable(meh ${SOURCES})
endif()

target_link_libraries(meh Qt5::Widgets)
