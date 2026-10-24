cmake_minimum_required(VERSION 3.21)
project(Verste VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

qt_add_executable(Verste
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/codeeditor.h
    src/codeeditor.cpp
    src/luauhighlighter.h
    src/luauhighlighter.cpp
    src/scripthistory.h
    src/scripthistory.cpp
    src/apicatalog.h
    src/apicatalog.cpp
)

target_link_libraries(Verste PRIVATE Qt6::Widgets)
