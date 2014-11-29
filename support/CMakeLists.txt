include_directories("${PROJECT_SOURCE_DIR}/support")

set(SOURCES
    conf/options.cc
    parsers/mcc.cc
    parsers/ndr.cc
    parsers/net.cc
    parsers/nupn.cc
    parsers/parse_error.cc
    parsers/parse_pn.cc
    parsers/parse_properties.cc
    parsers/pnml.cc
    pn/module.cc
    pn/net.cc
    pn/place.cc
    pn/transition.cc
    util/paths.cc
    )

add_library(support STATIC ${SOURCES})
