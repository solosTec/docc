# 
#	reset 
#
set (docruntime_lib)

set (docruntime_cpp
     ${PROJECT_SOURCE_DIR}/src/toc.cpp
     ${PROJECT_SOURCE_DIR}/src/currency.cpp
)
    
set (docruntime_h
     ${PROJECT_SOURCE_DIR}/include/toc.h
     ${PROJECT_SOURCE_DIR}/include/currency.h
)

set (docruntime_lib
  ${docruntime_cpp}
  ${docruntime_h}
)
