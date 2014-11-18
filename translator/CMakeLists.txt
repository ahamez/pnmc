include_directories("${PROJECT_SOURCE_DIR}/translator")

set(SOURCES
    main.cc
    fill_configuration.cc
    tina.cc
    )

add_executable(translator ${SOURCES})
target_link_libraries(translator support ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
