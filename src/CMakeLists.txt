set(SOURCES
    main.cc
    conf/fill_configuration.cc
    mc/classic/count_tokens.cc
    mc/classic/dump.cc
    mc/classic/firing_rule.cc
    mc/classic/make_order.cc
    mc/classic/worker.cc
    mc/mc.cc
    parsers/bpn.cc
    parsers/helpers.cc
    parsers/parse_error.cc
    parsers/parse.cc
    parsers/pnml.cc
    parsers/prod.cc
    parsers/tina.cc
    parsers/xml.cc
    pn/module.cc
    pn/net.cc
    pn/place.cc
    pn/tina.cc
    pn/transition.cc
    util/export_to_tina.cc
    util/paths.cc
    )

add_executable(pnmc ${SOURCES})
target_link_libraries(pnmc ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS pnmc DESTINATION bin)
