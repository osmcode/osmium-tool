#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool
#
#-----------------------------------------------------------------------------

file(GLOB OSMIUM_SOURCE_FILES *.cpp)

add_executable(osmium ${OSMIUM_SOURCE_FILES})

target_link_libraries(osmium ${Boost_LIBRARIES} ${CRYPTOPP_LIBRARIES} ${OSMIUM_LIBRARIES})

install(TARGETS osmium DESTINATION bin)

