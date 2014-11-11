include_directories("${PROJECT_SOURCE_DIR}/pnmc")

set(SOURCES
    main.cc
    conf/fill_configuration.cc
    mc/classic/count_tokens.cc
    mc/classic/dead_states.cc
    mc/classic/firing_rule.cc
    mc/classic/make_order.cc
    mc/classic/path_to.cc
    mc/classic/threads.cc
    mc/classic/worker.cc
    mc/shared/dump.cc
    mc/mc.cc
    )

add_executable(pnmc ${SOURCES})
target_link_libraries(pnmc shared ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES})
install(TARGETS pnmc DESTINATION bin)
