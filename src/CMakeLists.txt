
file(GLOB OSMIUM_SOURCE_FILES *.cpp)

add_executable(osmium ${OSMIUM_SOURCE_FILES})

target_link_libraries(osmium ${LIBRARIES})

install(TARGETS osmium DESTINATION bin)

