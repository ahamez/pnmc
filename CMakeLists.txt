################################################################################

cmake_minimum_required (VERSION 2.8.10)

################################################################################

project (pnmc)

option (PACKED "Pack structures" OFF)
option (PROFILE "Profile the library" OFF)
option (LIBSDD "Path to libsdd")
option (NO_COROUTINE "Don't use Boost.Coroutine" OFF)

################################################################################

set(Boost_USE_STATIC_LIBS ON)
if (NOT NO_COROUTINE)
  find_package(Boost 1.55.0 REQUIRED COMPONENTS program_options coroutine context system)
else ()
  find_package(Boost 1.55.0 REQUIRED COMPONENTS program_options)
endif ()
message("-- Boost Include: ${Boost_INCLUDE_DIR}")
message("-- Boost Libraries: ${Boost_LIBRARY_DIRS}")

################################################################################

if (PACKED)
  add_definitions("-DLIBSDD_PACKED")
endif ()

if (PROFILE)
  add_definitions("-DLIBSDD_PROFILE")
endif ()

################################################################################

if (NO_COROUTINE)
  add_definitions("-DHAS_NO_BOOST_COROUTINE")
endif ()

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions("-Wall")
add_definitions("-std=c++11")
add_definitions("-ftemplate-depth=256")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions("-stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -stdlib=libc++")
  set(CMAKE_CXX_FLAGS_RELEASE "-O4")
endif ()

set( CMAKE_CXX_FLAGS_RELEASE
     "${CMAKE_CXX_FLAGS_RELEASE} -fstrict-aliasing -DNDEBUG")

set( CMAKE_CXX_FLAGS_DEBUG
     "${CMAKE_CXX_FLAGS_DEBUG} -O0")

################################################################################

include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${LIBSDD}")

################################################################################

add_subdirectory(src)

################################################################################
