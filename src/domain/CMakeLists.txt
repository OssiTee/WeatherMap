add_library(domain
    weather/WeatherService.cpp
    city/CityService.cpp
    map/MapService.cpp
    CoordinateNormalizer.cpp
)

target_include_directories(domain PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(domain PUBLIC
)
