include_directories("${PROJECT_SOURCE_DIR}/shared")

set(SOURCES
    conf/options.cc
    parsers/ndr.cc
    parsers/net.cc
    parsers/nupn.cc
    parsers/parse_error.cc
    parsers/parse.cc
    parsers/pnml.cc
    pn/module.cc
    pn/net.cc
    pn/place.cc
    pn/transition.cc
    util/paths.cc
    )

add_library(shared STATIC ${SOURCES})
