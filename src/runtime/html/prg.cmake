# 
#	reset 
#
set (doc2html_prg)

set (doc2html_cpp
    
    ${PROJECT_SOURCE_DIR}/src/runtime/html/main.cpp
    ${PROJECT_SOURCE_DIR}/src/runtime/html/controller.cpp
)
    
set (doc2html_h
    ${PROJECT_SOURCE_DIR}/src/runtime/html/controller.h
)

set (doc2html_prg
  ${doc2html_cpp}
  ${doc2html_h}
)

