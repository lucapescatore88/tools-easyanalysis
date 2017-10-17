file(GLOB ToolsHeaders "*.hpp")
file(GLOB ToolsSources "*.cpp")

# You want to include the roofit folder to compile this code
include_directories(${EASYTOOLS_ROOFITPATH})

# Generate dictionary
ROOT_GENERATE_DICTIONARY(G__lib${TOOLS_LIB_NAME} ${ToolsHeaders} LINKDEF ${LINKDEF_TOOLS})

# Create library
add_library(${TOOLS_LIB_NAME} STATIC ${ToolsSources} G__lib${TOOLS_LIB_NAME}.cxx)

# Link library to ROOFIT (auto-load dependency to ROOT)
target_link_libraries(${TOOLS_LIB_NAME} PUBLIC ${ROOFIT_LIB_NAME})
