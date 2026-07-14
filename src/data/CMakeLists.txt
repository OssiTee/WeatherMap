add_library(data
    weather/WeatherRepository.cpp
    weather/NetworkClient.cpp
    weather/FmiXmlParser.cpp
    weather/WeatherCache.cpp
    cloud/CloudCoverageRepository.cpp
    city/CityLoader.cpp
    map/GeoJsonFileProvider.cpp
)

target_include_directories(data PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(data PUBLIC
    domain
    Qt6::Core
    Qt6::Network
    spdlog::spdlog
)
