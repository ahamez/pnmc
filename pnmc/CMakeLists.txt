include_directories("${PROJECT_SOURCE_DIR}/pnmc")

set(SOURCES
  conf/configuration.hh
  conf/default.hh
  conf/fill_configuration.cc
  conf/fill_configuration.hh
  mc/classic/advance.hh
  mc/classic/advance_capped.hh
  mc/classic/bounded_post.hh
  mc/classic/count_tokens.cc
  mc/classic/count_tokens.hh
  mc/classic/dead_states.cc
  mc/classic/enabled.hh
  mc/classic/enabled_inhibitor.hh
  mc/classic/filter_ge.hh
  mc/classic/filter_lt.hh
  mc/classic/firing_rule.cc
  mc/classic/firing_rule.hh
  mc/classic/make_order.cc
  mc/classic/make_order.hh
  mc/classic/path_to.hh
  mc/classic/path_to.cc
  mc/classic/place_bound.cc
  mc/classic/post.hh
  mc/classic/pre.hh
  mc/classic/pre_clock.hh
  mc/classic/reachability.cc
  mc/classic/reachability.hh
  mc/classic/reachability_ast.cc
  mc/classic/reachability_ast.hh
  mc/classic/reachability_eval.cc
  mc/classic/reachability_eval.hh
  mc/classic/sdd.hh
  mc/classic/set.hh
  mc/classic/threads.cc
  mc/classic/classic.cc
  mc/shared/exceptions.hh
  mc/shared/export.hh
  mc/shared/export_configuration.hh
  mc/shared/interruptible.hh
  mc/shared/live.hh
  mc/shared/results.hh
  mc/shared/results_serialize.hh
  mc/shared/statistics.hh
  mc/shared/statistics_configuration.hh
  mc/shared/statistics_serialize.hh
  mc/shared/step.hh
  mc/mc.cc
  mc/mc.hh
  mc/mc_impl.hh
  main.cc
  )

add_executable(pnmc ${SOURCES})
target_link_libraries(pnmc support ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS pnmc DESTINATION bin)
