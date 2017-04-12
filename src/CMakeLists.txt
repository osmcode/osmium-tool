#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool
#
#-----------------------------------------------------------------------------

file(GLOB OSMIUM_SOURCE_FILES *.cpp */*.cpp ${PROJECT_BINARY_DIR}/src/version.cpp)

add_executable(osmium ${OSMIUM_SOURCE_FILES})

target_link_libraries(osmium ${Boost_LIBRARIES} ${OSMIUM_LIBRARIES})

set_pthread_on_target(osmium)

install(TARGETS osmium DESTINATION bin)

