file(GLOB RooPdfsHeaders "*.h")
file(GLOB RooPdfsSources "*.cpp")

# Generate Root dictionary from roofit folder
ROOT_GENERATE_DICTIONARY(G__${ROOFIT_LIB_NAME} ${RooPdfsHeaders} LINKDEF ${LINKDEF_ROOFIT})

# Create library
add_library(${ROOFIT_LIB_NAME} STATIC ${RooPdfsSources} G__${ROOFIT_LIB_NAME}.cxx)

# Link library to ROOT
target_link_libraries(${ROOFIT_LIB_NAME} PUBLIC ${ROOT_LIBRARIES})
