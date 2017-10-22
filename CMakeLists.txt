cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1)

project(easyanalysis)

set(PROJECT_SYS ${CMAKE_CURRENT_SOURCE_DIR})

set(ROOFIT_PATH ${PROJECT_SYS}/roofit)
set(TOOLS_PATH  ${PROJECT_SYS}/tools)

set(LINKDEF_ROOFIT ${PROJECT_SYS}/LinkDef_roofit.h)
set(LINKDEF_TOOLS  ${PROJECT_SYS}/LinkDef_tools.h)

set(ROOFIT_NAME roofit)
set(TOOLS_NAME  tools)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SYS}/lib)

set(DIRECTORY_GSL $ENV{GSLSYS}/include)

message(STATUS "CMaking ${PROJECT_NAME}")
message(STATUS "Sources ${ROOFIT_PATH}")
message(STATUS "Sources ${TOOLS_PATH}")
message(STATUS "Library ${LIBRARY_OUTPUT_PATH}")

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Cling TreePlayer Tree Rint MathMore MathCore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA XMLIO MLP)
include(${ROOT_USE_FILE})

include_directories(${PROJECT_SYS} ${ROOT_INCLUDE_DIRS} ${DIRECTORY_GSL})
add_definitions(${ROOT_CXX_FLAGS})

add_subdirectory(${ROOFIT_PATH})
add_subdirectory(${TOOLS_PATH})
