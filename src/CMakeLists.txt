set(SOURCES
    main.cc
    conf/fill_configuration.cc
    mc/classic/dead.cc
    mc/classic/dump.cc
    mc/classic/live.cc
    mc/classic/make_order.cc
    mc/classic/post.cc
    mc/classic/pre.cc
    mc/classic/worker.cc
    mc/mc.cc
    parsers/bpn.cc
    parsers/helpers.cc
    parsers/parse_error.cc
    parsers/parse.cc
    parsers/prod.cc
    parsers/tina.cc
    parsers/xml.cc
    pn/module.cc
    pn/net.cc
    pn/place.cc
    pn/transition.cc
    )

add_executable(pnmc ${SOURCES})
target_link_libraries(pnmc ${Boost_LIBRARIES})
install(TARGETS pnmc DESTINATION bin)
