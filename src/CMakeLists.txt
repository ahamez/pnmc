set(SOURCES
    main.cc
    conf/configuration.cc
    conf/fill_configuration.cc
    mc/post.cc
    mc/pre.cc
    mc/work.cc
    parsers/parse.cc
    parsers/prod.cc
    parsers/tina.cc
    parsers/xml.cc
    pn/arc.cc
    pn/module.cc
    pn/net.cc
    pn/place.cc
    pn/transition.cc
    )

add_executable(pnmc ${SOURCES})
target_link_libraries(pnmc ${Boost_LIBRARIES})
