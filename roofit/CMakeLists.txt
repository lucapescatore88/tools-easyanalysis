file(GLOB RooPdfsHeaders "*.h")
file(GLOB RooPdfsSources "*.cpp")

#Generate Root dictionary from roofit folder
ROOT_GENERATE_DICTIONARY(G__RooFitTools ${RooPdfsHeaders} LINKDEF  ${LINKDEF_ROOFIT})
#Create actual library
add_library(${ROOFIT_LIB_NAME} SHARED ${RooPdfsSources} G__RooFitTools.cxx)
#Link library to ROOT ones
target_link_libraries(${ROOFIT_LIB_NAME} PUBLIC ${ROOT_LIBRARIES})




