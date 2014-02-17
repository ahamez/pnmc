set(SOURCES
    main.cc
    conf/fill_configuration.cc
    mc/bounded_post.cc
    mc/dead.cc
    mc/live.cc
    mc/post.cc
    mc/pre.cc
    mc/work.cc
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
