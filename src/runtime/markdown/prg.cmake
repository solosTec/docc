# 
#	reset 
#
set (doc2md_prg)

set (doc2md_cpp
    
    ${PROJECT_SOURCE_DIR}/src/runtime/markdown/main.cpp
    ${PROJECT_SOURCE_DIR}/src/runtime/markdown/controller.cpp
)
    
set (doc2md_h
    ${PROJECT_SOURCE_DIR}/src/runtime/markdown/controller.h
)

set (doc2md_prg
  ${doc2md_cpp}
  ${doc2md_h}
)

