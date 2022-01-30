#-----------------------------------------------------------------------------
#
#  Create man pages
#
#-----------------------------------------------------------------------------
message(STATUS "Looking for pandoc")
find_program(PANDOC pandoc)

function(add_man_page _section _name)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man${_section})
    set(_output_file ${CMAKE_CURRENT_BINARY_DIR}/man${_section}/${_name}.${_section})
    install(FILES ${_output_file} DESTINATION share/man/man${_section})
    set(_source_file ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.md)
    set(_dest_file ${CMAKE_CURRENT_BINARY_DIR}/source/${_name}.md)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/common-options.md MAN_COMMON_OPTIONS)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/progress-options.md MAN_PROGRESS_OPTIONS)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/input-options.md MAN_INPUT_OPTIONS)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/output-options.md MAN_OUTPUT_OPTIONS)
    file(READ ${CMAKE_SOURCE_DIR}/export-example-config/default-config.json EXPORT_DEFAULT_CONFIG)
    configure_file(${_source_file} ${_dest_file} @ONLY)
    string(TOUPPER ${_name} _name_upcase)
    add_custom_command(OUTPUT ${_output_file}
        COMMAND ${PANDOC}
            ${PANDOC_MAN_OPTIONS}
            --variable "title=${_name_upcase}"
            --variable "section=${_section}"
            -o ${_output_file}
            ${_dest_file}
        DEPENDS ${_source_file} manpage.template common-options.md input-options.md output-options.md
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building manpage ${_name}.${_section}"
        VERBATIM)
    set(ALL_MAN_PAGES "${ALL_MAN_PAGES};${_output_file}" PARENT_SCOPE)
endfunction()


if(PANDOC)
    message(STATUS "Looking for pandoc - found")
    message(STATUS "  Manual pages will be built")
    set(PANDOC_MAN_OPTIONS
        -s
        -t man
        --template ${CMAKE_CURRENT_SOURCE_DIR}/manpage.template
        --variable "description=osmium/${PROJECT_VERSION}"
        --variable "version=${PROJECT_VERSION}"
        --variable "author=${AUTHOR}"
    )

    add_man_page(1 osmium)
    add_man_page(1 osmium-add-locations-to-ways)
    add_man_page(1 osmium-apply-changes)
    add_man_page(1 osmium-cat)
    add_man_page(1 osmium-changeset-filter)
    add_man_page(1 osmium-check-refs)
    add_man_page(1 osmium-create-locations-index)
    add_man_page(1 osmium-derive-changes)
    add_man_page(1 osmium-diff)
    add_man_page(1 osmium-export)
    add_man_page(1 osmium-extract)
    add_man_page(1 osmium-fileinfo)
    add_man_page(1 osmium-getid)
    add_man_page(1 osmium-getparents)
    add_man_page(1 osmium-merge)
    add_man_page(1 osmium-merge-changes)
    add_man_page(1 osmium-query-locations-index)
    add_man_page(1 osmium-removeid)
    add_man_page(1 osmium-renumber)
    add_man_page(1 osmium-show)
    add_man_page(1 osmium-sort)
    add_man_page(1 osmium-tags-count)
    add_man_page(1 osmium-tags-filter)
    add_man_page(1 osmium-time-filter)
    add_man_page(5 osmium-file-formats)
    add_man_page(5 osmium-index-types)
    add_man_page(5 osmium-output-headers)

    add_custom_target(man ALL DEPENDS ${ALL_MAN_PAGES})
else()
    message(STATUS "Looking for pandoc - not found")
    message(STATUS "  Manual pages will not be built")
endif()


