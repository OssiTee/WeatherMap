cmake_minimum_required(VERSION 3.16)
project(WeatherMap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt automation features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Locate Qt6 (Test module is handled in the tests directory)
find_package(Qt6 REQUIRED COMPONENTS
    Widgets Network Core Svg SvgWidgets Concurrent
)

# Third‑party library: spdlog
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    FetchContent_MakeAvailable(spdlog)
    if(NOT TARGET spdlog::spdlog AND TARGET spdlog)
        add_library(spdlog::spdlog ALIAS spdlog)
    endif()
endif()

# ----------------------------------------------------------------------
# Subdirectories (Project layers)
# ----------------------------------------------------------------------
add_subdirectory(src/domain)
add_subdirectory(src/data)
add_subdirectory(src/viewmodel)
add_subdirectory(src/view)

# ----------------------------------------------------------------------
# Main application
# ----------------------------------------------------------------------
qt_add_executable(WeatherMap
    main.cpp
    resources/map.qrc
    resources/icons.qrc
    resources/cities.qrc
    resources/queries.fmi.qrc
)

target_include_directories(WeatherMap PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(WeatherMap PRIVATE
    domain
    data
    viewmodel
    view
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Concurrent
    spdlog::spdlog
)

# Place the final binary into the bin/ directory
set_target_properties(WeatherMap PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ----------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
