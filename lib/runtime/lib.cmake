# 
#	reset 
#
set (docruntime_lib)

set (docruntime_cpp
     ${PROJECT_SOURCE_DIR}/src/numbering.cpp
)
    
set (docruntime_h
     ${PROJECT_SOURCE_DIR}/include/numbering.h
)

set (docruntime_lib
  ${docruntime_cpp}
  ${docruntime_h}
)

