# 
#	reset 
#
set (dom_lib)

set (dom_cpp
     ${PROJECT_SOURCE_DIR}/src/dom.cpp
     ${PROJECT_SOURCE_DIR}/src/formatting.cpp
)
    
set (dom_h
     ${PROJECT_SOURCE_DIR}/include/html/dom.hpp
     ${PROJECT_SOURCE_DIR}/include/html/formatting.h
)

set (dom_lib
    ${dom_cpp}
    ${dom_h}
)

