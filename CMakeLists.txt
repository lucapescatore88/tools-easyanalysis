cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(easytools)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Cling TreePlayer Tree Rint MathMore MathCore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA XMLIO MLP)
include(${ROOT_USE_FILE})

set(PROJECT_SYS ${CMAKE_CURRENT_SOURCE_DIR})

set(EASYTOOLS_ROOFITPATH ${PROJECT_SYS}/roofit)
set(EASYTOOLS_TOOLSPATH ${PROJECT_SYS}/tools)

set(LINKDEF_ROOFIT ${PROJECT_SYS}/LinkDef_roofit.h)
set(LINKDEF_TOOLS ${PROJECT_SYS}/LinkDef_tools.h)

set(ROOFIT_LIB_NAME toolsroofit)
set(TOOLS_LIB_NAME tools)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SYS}/lib)
set(DIRECTORY_GSL $ENV{GSLSYS}/include)

message("EasyTools PROJECT_SYS is set to" ${PROJECT_SYS})
include_directories(${PROJECT_SYS} ${ROOT_INCLUDE_DIRS} ${DIRECTORY_GSL})
add_definitions(${ROOT_CXX_FLAGS})

add_subdirectory(${EASYTOOLS_ROOFITPATH})
add_subdirectory(${EASYTOOLS_TOOLSPATH})
