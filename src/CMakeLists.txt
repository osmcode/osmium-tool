#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool
#
#-----------------------------------------------------------------------------

add_executable(osmium
    main.cpp
    commands.cpp
    ${PROJECT_BINARY_DIR}/src/version.cpp
    ${OSMIUM_SOURCE_FILES}
)

target_link_libraries(osmium ${Boost_LIBRARIES} ${OSMIUM_LIBRARIES})

set_pthread_on_target(osmium)

install(TARGETS osmium DESTINATION bin)

