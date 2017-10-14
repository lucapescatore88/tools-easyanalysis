cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(easytools_analysis)


list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Cling TreePlayer Tree Rint MathMore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA  XMLIO MLP)
include(${ROOT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})

add_subdirectory(roofit)
add_subdirectory(tools)


