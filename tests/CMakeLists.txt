find_package(PythonInterp 3.3)
if (PYTHONINTERP_FOUND)
  message("-- Python interpreter: ${PYTHON_EXECUTABLE}")
  add_test(NAME test_models
           COMMAND ${PYTHON_EXECUTABLE}
                   ${PROJECT_SOURCE_DIR}/tests/test_models/test_models.py
                   $<TARGET_FILE:pnmc>
                   ${pnmc_BINARY_DIR}/test_models_output
          )
endif ()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
if (COVERAGE AND CMAKE_BUILD_TYPE MATCHES Debug)
  set(LCOV_EXCLUDE "'/boost/*'" "'/c++/*'" "'pnmc/external/*'" "'libsdd/*'")
  setup_target_for_coverage(coverage ${PYTHON_EXECUTABLE} coverage "${PROJECT_SOURCE_DIR}/tests/test_models/test_models.py;$<TARGET_FILE:pnmc>;${pnmc_BINARY_DIR}/test_models_output")
endif()
