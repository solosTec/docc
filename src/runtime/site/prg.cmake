# 
#	reset 
#
set (doc2site_prg)

set (doc2site_cpp
    
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/initialize.cpp
    ${PROJECT_SOURCE_DIR}/src/build.cpp
    ${PROJECT_SOURCE_DIR}/src/page.cpp
    ${PROJECT_SOURCE_DIR}/src/footer.cpp
    ${PROJECT_SOURCE_DIR}/src/navbar.cpp
    ${PROJECT_SOURCE_DIR}/src/generator.cpp
    ${PROJECT_SOURCE_DIR}/src/download.cpp
)
    
set (doc2site_h
    ${CMAKE_BINARY_DIR}/include/site_defs.h
    ${PROJECT_SOURCE_DIR}/src/initialize.h
    ${PROJECT_SOURCE_DIR}/src/build.h
    ${PROJECT_SOURCE_DIR}/src/page.h
    ${PROJECT_SOURCE_DIR}/src/footer.h
    ${PROJECT_SOURCE_DIR}/src/navbar.h
    ${PROJECT_SOURCE_DIR}/src/generator.h
    ${PROJECT_SOURCE_DIR}/src/download.h
    ${PROJECT_SOURCE_DIR}/src/details/root_certificates.hpp
)

set (doc2site_prg
  ${doc2site_cpp}
  ${doc2site_h}
)

