find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Gui)

qt_add_library(viewmodel STATIC
    weather/WeatherDataViewModel.cpp
    cloud/CloudOverlayViewModel.cpp
    city/CityLabelsViewModel.cpp
    map/MapLoaderViewModel.cpp

    ${CMAKE_SOURCE_DIR}/include/viewmodel/weather/WeatherDataViewModel.h
    ${CMAKE_SOURCE_DIR}/include/viewmodel/cloud/CloudOverlayViewModel.h
    ${CMAKE_SOURCE_DIR}/include/viewmodel/city/CityLabelsViewModel.h
    ${CMAKE_SOURCE_DIR}/include/viewmodel/map/MapLoaderViewModel.h
)

target_include_directories(viewmodel PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(viewmodel PUBLIC
    Qt6::Core
    Qt6::Concurrent
    Qt6::Gui
    spdlog::spdlog
    domain
)
