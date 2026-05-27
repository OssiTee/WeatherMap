find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

qt_add_library(view STATIC
    MainWindow.cpp
    MapWidget.cpp
    ${CMAKE_SOURCE_DIR}/include/view/MainWindow.h
    ${CMAKE_SOURCE_DIR}/include/view/MapWidget.h
)

target_include_directories(view PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(view PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    viewmodel
)
