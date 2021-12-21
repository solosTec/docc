# 
#	reset 
#
set (tex_lib)

set (tex_cpp
     ${PROJECT_SOURCE_DIR}/src/tex.cpp
     ${PROJECT_SOURCE_DIR}/src/formatting.cpp
)
    
set (tex_h
     ${PROJECT_SOURCE_DIR}/include/tex/tex.hpp
     ${PROJECT_SOURCE_DIR}/include/tex/formatting.h
)


set (tex_lib
    ${tex_cpp}
    ${tex_h}
)

