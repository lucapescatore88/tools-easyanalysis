cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(easytools_analysis)


list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Cling TreePlayer Tree Rint MathMore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA  XMLIO MLP)
include(${ROOT_USE_FILE})
set(PROJECT_SYS ${CMAKE_CURRENT_SOURCE_DIR})
set(LINKDEF_TOOLS ${PROJECT_SYS}/LinkDef.h)
set(LINKDEF_ROOFIT ${PROJECT_SYS}/Roofit_LinkDef.h)
set(EASYTOOLS_ROOFITPATH ${PROJECT_SYS}/roofit)
set(EASYTOOLS_TOOLSPATH ${PROJECT_SYS}/tools)


set(LIBRARY_OUTPUT_PATH ${PROJECT_SYS}/lib)

message("EasyTools PROJECT_SYS is set to" ${PROJECT_SYS})
include_directories(${PROJECT_SYS} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})

set(TOOLS_LIB_NAME tools)
set(ROOFIT_LIB_NAME roofittools)


add_subdirectory(${EASYTOOLS_ROOFITPATH})
add_subdirectory(${EASYTOOLS_TOOLSPATH})

