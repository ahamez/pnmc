################################################################################

cmake_minimum_required (VERSION 2.8.10)

################################################################################

project (pnmc)

################################################################################

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.54.0 REQUIRED COMPONENTS program_options context coroutine)
message("-- Boost Include: ${Boost_INCLUDE_DIR}")
message("-- Boost Libraries: ${Boost_LIBRARY_DIRS}")

################################################################################

option (PACKED "Pack structures" OFF)
option (PROFILE "Profile the library" OFF)
option (LIBSDD "Path to libsdd")

if (PACKED)
  add_definitions("-DLIBSDD_PACKED")
endif ()

if (PROFILE)
  add_definitions("-DLIBSDD_PROFILE")
endif ()

################################################################################

add_definitions("-Wall")
add_definitions("-std=c++11")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions("-stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -stdlib=libc++")
  set(CMAKE_CXX_FLAGS_RELEASE "-O4 -ftemplate-depth=256")
endif ()

set( CMAKE_CXX_FLAGS_RELEASE
     "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden -fstrict-aliasing -DNDEBUG")

set( CMAKE_CXX_FLAGS_DEBUG
     "${CMAKE_CXX_FLAGS_DEBUG} -O0")

################################################################################

include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${LIBSDD}")

################################################################################

add_subdirectory(src)

################################################################################
