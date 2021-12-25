# 
#	reset 
#
set (doc2site_prg)

set (doc2site_cpp
    
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/initialize.cpp
)
    
set (doc2site_h
    ${CMAKE_BINARY_DIR}/include/site_defs.h
    ${PROJECT_SOURCE_DIR}/src/initialize.h
)

set (doc2site_prg
  ${doc2site_cpp}
  ${doc2site_h}
)

