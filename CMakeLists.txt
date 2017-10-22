cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1)

project(easyanalysis)

set(TOOLSSYS $ENV{TOOLSSYS})

set(ROOFIT_NAME roofit)
set(TOOLS_NAME  tools)

set(ROOFIT_PATH ${TOOLSSYS}/${ROOFIT_NAME})
set(TOOLS_PATH  ${TOOLSSYS}/${TOOLS_NAME})

set(ROOFIT_LINKDEF ${ROOFIT_PATH}/LinkDef.h)
set(TOOLS_LINKDEF  ${TOOLS_PATH}/LinkDef.h)

set(LIBRARY_OUTPUT_PATH ${TOOLSSYS}/lib)
set(TOOLSLIB_PATH ${LIBRARY_OUTPUT_PATH})

set(GSLSYS $ENV{GSLSYS}/include)

get_directory_property(HASPARENTSCOPE PARENT_DIRECTORY)
if(HASPARENTSCOPE)
    set(ROOFIT_NAME roofit PARENT_SCOPE)
    set(TOOLS_NAME  tools  PARENT_SCOPE)

    set(ROOFIT_PATH ${TOOLSSYS}/${ROOFIT_NAME} PARENT_SCOPE)
    set(TOOLS_PATH  ${TOOLSSYS}/${TOOLS_NAME}  PARENT_SCOPE)

    set(TOOLSLIB_PATH ${LIBRARY_OUTPUT_PATH} PARENT_SCOPE)
endif()

message(STATUS "CMaking ${PROJECT_NAME}")
message(STATUS "Sources ${ROOFIT_PATH}")
message(STATUS "Sources ${TOOLS_PATH}")
message(STATUS "Library ${TOOLSLIB_PATH}")

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED Cling TreePlayer Tree Rint MathMore MathCore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA XMLIO MLP)
include(${ROOT_USE_FILE})

include_directories(${TOOLSSYS} ${TOOLS_PATH} ${ROOFIT_PATH} ${ROOT_INCLUDE_DIRS} ${GSLSYS})
add_definitions(${ROOT_CXX_FLAGS})

add_subdirectory(${ROOFIT_PATH})
add_subdirectory(${TOOLS_PATH})
