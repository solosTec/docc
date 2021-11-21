# 
#	reset 
#
set (docruntime_lib)

set (docruntime_cpp
     ${PROJECT_SOURCE_DIR}/src/toc.cpp
     ${PROJECT_SOURCE_DIR}/src/currency.cpp
     ${PROJECT_SOURCE_DIR}/src/i18n.cpp
     ${PROJECT_SOURCE_DIR}/src/stream.cpp
)
    
set (docruntime_h
     ${PROJECT_SOURCE_DIR}/include/rt/toc.h
     ${PROJECT_SOURCE_DIR}/include/rt/currency.h
     ${PROJECT_SOURCE_DIR}/include/rt/i18n.h
     ${PROJECT_SOURCE_DIR}/include/rt/stream.h
)

set (docruntime_lib
  ${docruntime_cpp}
  ${docruntime_h}
)

