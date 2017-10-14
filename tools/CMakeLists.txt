

file(GLOB ToolsHeaders "*.hpp")
file(GLOB ToolsSources "*.cpp")
set(LIBRARY_OUTPUT_DIRECTORY ./../lib)
set(LIBRARY_OUTPUT_PATH ./../lib)

include_directories(./../roofit)
ROOT_GENERATE_DICTIONARY(G__libtools ${ToolsHeaders} LINKDEF  ./../LinkDef.h)
add_library(libtools SHARED ${ToolsSources} G__libtools.cxx)
target_link_libraries(libtools PUBLIC RooFitTools)

