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

add_executable(meh
    main.cpp
    resources.qrc
    buffer.cpp
    command.cpp
    completer.cpp
    editor.cpp
    fileslookup.cpp
    grep.cpp
    lsp.cpp
    lsp_manager.cpp
    lsp/lsp_clangd.cpp
    lsp/lsp_gopls.cpp
    normal.cpp
    submode.cpp
    syntax.cpp
    visual.cpp
    window.cpp
)

target_link_libraries(meh Qt5::Widgets)