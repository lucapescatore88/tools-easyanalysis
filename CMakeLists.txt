set(PROJECT easyanalysis)
message(STATUS "Project " ${PROJECT})

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(PROJECT VERSION 1.0)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

if(NOT GSL_FOUND)
    message(STATUS "Detecting GSL")
    set(GSL_ROOT_DIR "$ENV{GSLSYS}")
    find_package(GSL REQUIRED)
    if(GSL_FOUND)
        message(STATUS "GSL_INCLUDES  = " ${GSL_INCLUDE_DIRS})
        message(STATUS "GSL_LIBRARIES = " ${GSL_LIBRARIES})
        include_directories(${GSL_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "GSL found " ${GSL_FOUND})
    endif()
endif()

if(NOT ROOT_FOUND)
    message(STATUS "Detecting ROOT")
    find_package(ROOT REQUIRED Cling TreePlayer Tree Rint MathMore MathCore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA XMLIO MLP)
    if(ROOT_FOUND)
        message(STATUS "ROOT_INCLUDES  = " ${ROOT_INCLUDE_DIRS})
        message(STATUS "ROOT_LIBRARIES = " ${ROOT_LIBRARIES})
        include_directories(${ROOT_INCLUDE_DIRS})
        include(${ROOT_USE_FILE})
    else()
        message(FATAL_ERROR "ROOT found " ${ROOT_FOUND})
    endif()
endif()

#Declare function to generate dictionaries!
function(GENERATE_DICTIONARY dictionary)
    if(${ROOT_VERSION} LESS 6.0)
        ROOT_GENERATE_DICTIONARY(${dictionary} ${ARGN})
    else()
        ROOT_GENERATE_DICTIONARY(${dictionary} MODULE ${dictionary} ${ARGN})
    endif()
endfunction()

set(ROOFIT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/roofit)
set(TOOLS_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/tools)

set(ROOFIT_LINKDEF ${ROOFIT_PATH}/LinkDef.h)
set(TOOLS_LINKDEF  ${TOOLS_PATH}/LinkDef.h)

include_directories(${ROOFIT_PATH} ${TOOLS_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

#Define headers and sources to compile for roofit and tools
file(GLOB sources_roofit ${ROOFIT_PATH}/*.cpp)
file(GLOB headers_roofit ${ROOFIT_PATH}/*.h)
file(GLOB sources_tools  ${TOOLS_PATH}/*.cpp)
file(GLOB headers_tools  ${TOOLS_PATH}/*.hpp)

#Remove from headers the linkdef file
list(REMOVE_ITEM headers_roofit ${ROOFIT_PATH}/LinkDef.h)
list(REMOVE_ITEM headers_tools  ${TOOLS_PATH}/LinkDef.h)

#Generate the roofit dictionaries
GENERATE_DICTIONARY(roofitDict ${headers_roofit} LINKDEF ${ROOFIT_PATH}/LinkDef.h)
add_library(roofit-objlib OBJECT ${sources_roofit} roofitDict.cxx)

#Generate the tools dictionaries
GENERATE_DICTIONARY(toolsDict ${headers_tools} LINKDEF ${TOOLS_PATH}/LinkDef.h)
add_library(tools-objlib OBJECT ${sources_tools} toolsDict.cxx)

add_library(${PROJECT}-shared SHARED $<TARGET_OBJECTS:roofit-objlib> $<TARGET_OBJECTS:tools-objlib>)
add_library(${PROJECT}-static STATIC $<TARGET_OBJECTS:roofit-objlib> $<TARGET_OBJECTS:tools-objlib>)

target_link_libraries(${PROJECT}-shared PUBLIC ${ROOT_LIBRARIES} ${GSL_LIBRARIES})
target_link_libraries(${PROJECT}-static PUBLIC ${ROOT_LIBRARIES} ${GSL_LIBRARIES})

if (NOT ${ROOT_VERSION} VERSION_LESS "6.0.0")
    install(FILES
            ${CMAKE_BINARY_DIR}/lib${PROJECT}Dict_rdict.pcm
            ${CMAKE_BINARY_DIR}/lib${PROJECT}Dict.rootmap
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif()

install(TARGETS ${PROJECT}-shared ${PROJECT}-static
		LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib
		ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib)
