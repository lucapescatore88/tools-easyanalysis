file(GLOB RooPdfsHeaders "*.h")
file(GLOB RooPdfsSources "*.cpp")

set(LIBRARY_OUTPUT_DIRECTORY ./../lib)
set(LIBRARY_OUTPUT_PATH ./../lib)

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})

ROOT_GENERATE_DICTIONARY(G__RooFitTools ${RooPdfsHeaders} LINKDEF  RooLinkPdfs.hpp)
add_library(RooFitTools SHARED ${RooPdfsSources} G__RooFitTools.cxx)
target_link_libraries(RooFitTools PUBLIC ${ROOT_LIBRARIES})

# ROOT_GENERATE_DICTIONARY(G__RooIpatia2 RooIpatia2.h LINKDEF  RooIpatia2LinkDef.h)
# add_library(RooIpatia2 SHARED RooIpatia2.cpp G__RooIpatia2.cxx)
# target_link_libraries(RooIpatia2 ${ROOT_LIBRARIES})



