# ======================================================================
#  WeatherMap – Unit Test Configuration
#  This file defines all test executables and their dependencies.
#  Each test is built as a standalone executable and registered in CTest.
# ======================================================================

enable_testing()

# ----------------------------------------------------------------------
# Qt6 Test framework
# ----------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Test)

# ----------------------------------------------------------------------
# Shared test resources (compiled into a generated .cpp file)
# ----------------------------------------------------------------------
qt_add_resources(test_resources
    ../resources/map.qrc
    ../resources/icons.qrc
    ../resources/cities.qrc
    ../resources/queries.fmi.qrc
)

# ----------------------------------------------------------------------
# List of all test source files
# ----------------------------------------------------------------------
set(TEST_SOURCES
    data/test_city_loader.cpp
    data/test_cloud_coverage_repository.cpp
    data/test_geojson_provider.cpp
    data/test_fmi_xml_parser.cpp
    data/test_weather_cache.cpp

    domain/test_city_service.cpp
    domain/test_cloud_service.cpp
    domain/test_coordinate_normalizer.cpp
    domain/test_weather_service.cpp
    domain/test_map_service.cpp

    view/test_map_widget.cpp

    viewmodel/test_city_labels.cpp
    viewmodel/test_cloud_overlay.cpp
    viewmodel/test_map_loader.cpp
    viewmodel/test_weather_data.cpp
)

# ----------------------------------------------------------------------
# Libraries required by all test executables
# ----------------------------------------------------------------------
set(TEST_LIBS
    Qt6::Test
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::Svg

    domain
    data
    viewmodel
    view

    spdlog::spdlog
)

# ----------------------------------------------------------------------
# Create one test executable per test source file
# ----------------------------------------------------------------------
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name}_exe
        ${test_src}
        ${test_resources}   # embed shared resources
    )

    set_target_properties(${test_name}_exe PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        AUTOMOC ON
        AUTOUIC ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )

    target_link_libraries(${test_name}_exe PRIVATE ${TEST_LIBS})

    add_test(NAME ${test_name}
             COMMAND ${CMAKE_BINARY_DIR}/tests/${test_name}_exe)
endforeach()
