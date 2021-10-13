# 
#	reset 
#
set (runtime_prg)

set (runtime_cpp
    
    ${PROJECT_SOURCE_DIR}/src/runtime/main.cpp
    ${PROJECT_SOURCE_DIR}/src/runtime/controller.cpp
)
    
set (runtime_h
    ${PROJECT_SOURCE_DIR}/src/runtime/controller.h
)

set (runtime_ast
)

source_group("ast" FILES ${runtime_ast})

set (runtime_prg
  ${runtime_cpp}
  ${runtime_h}
  ${runtime_ast}
)

