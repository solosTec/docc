# 
#	reset 
#
set (doc2LaTeX_prg)

set (doc2LaTeX_cpp
    
    ${PROJECT_SOURCE_DIR}/src/runtime/LaTeX/main.cpp
    ${PROJECT_SOURCE_DIR}/src/runtime/LaTeX/controller.cpp
)
    
set (doc2LaTeX_h
    ${PROJECT_SOURCE_DIR}/src/runtime/LaTeX/controller.h
)

set (doc2LaTeX_prg
  ${doc2LaTeX_cpp}
  ${doc2LaTeX_h}
)

