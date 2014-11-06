include_directories("${PROJECT_SOURCE_DIR}/pnmc")

set(SOURCES
    main.cc
    conf/fill_configuration.cc
    mc/classic/count_tokens.cc
    mc/classic/firing_rule.cc
    mc/classic/make_order.cc
    mc/classic/worker.cc
    mc/shared/dump.cc
    mc/mc.cc
    util/paths.cc
    ${PROJECT_SOURCE_DIR}/shared/parsers/bpn.cc
    ${PROJECT_SOURCE_DIR}/shared/parsers/helpers.cc
    ${PROJECT_SOURCE_DIR}/shared/parsers/parse_error.cc
    ${PROJECT_SOURCE_DIR}/shared/parsers/parse.cc
    ${PROJECT_SOURCE_DIR}/shared/parsers/pnml.cc
    ${PROJECT_SOURCE_DIR}/shared/parsers/tina.cc
    ${PROJECT_SOURCE_DIR}/shared/parsers/xml.cc
    ${PROJECT_SOURCE_DIR}/shared/pn/module.cc
    ${PROJECT_SOURCE_DIR}/shared/pn/net.cc
    ${PROJECT_SOURCE_DIR}/shared/pn/place.cc
    ${PROJECT_SOURCE_DIR}/shared/pn/transition.cc
    )

add_executable(pnmc ${SOURCES})
target_link_libraries(pnmc ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES})
install(TARGETS pnmc DESTINATION bin)
