cmake_minimum_required (VERSION 2.8.9)

################################################################################

project (pnmc)

option (LIBSDD_PATH "Path to libsdd")
option (PACKED "Pack structs in memory (experimental)" OFF)

################################################################################

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.55.0 REQUIRED COMPONENTS program_options coroutine context system thread filesystem)

if (NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif ()

message("-- Boost Include: ${Boost_INCLUDE_DIR}")
message("-- Boost Libraries: ${Boost_LIBRARY_DIRS}")

################################################################################

if (PACKED)
  add_definitions("-DLIBSDD_PACKED")
endif ()

################################################################################

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX14)
if(NOT COMPILER_SUPPORTS_CXX14)
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support.")
endif ()

################################################################################

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

find_path(LIBSDD_INCLUDE_DIR sdd/sdd.hh PATHS ${LIBSDD_PATH})
if (NOT LIBSDD_INCLUDE_DIR)
  message(FATAL_ERROR "libsdd not found")
endif ()

find_package (Threads)

add_definitions("-Wall")
add_definitions("-std=c++1y")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions("-stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++1y -stdlib=libc++")
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
  endif ()
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden -fvisibility-inlines-hidden")

################################################################################

include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/external/rapidxml")
include_directories("${PROJECT_SOURCE_DIR}/external/cereal/include")
include_directories("${LIBSDD_INCLUDE_DIR}")

################################################################################

add_subdirectory(support)
link_directories(${pnmc_BINARY_DIR}/support)
add_subdirectory(pnmc)
add_subdirectory(translator)

################################################################################
