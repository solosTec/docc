# 
#	reset 
#
set (docscript_lib)

set (docscript_cpp
     ${PROJECT_SOURCE_DIR}/src/symbol.cpp
     ${PROJECT_SOURCE_DIR}/src/token.cpp
     ${PROJECT_SOURCE_DIR}/src/tokenizer.cpp
     ${PROJECT_SOURCE_DIR}/src/sanitizer.cpp
     ${PROJECT_SOURCE_DIR}/src/utils.cpp
     ${PROJECT_SOURCE_DIR}/src/context.cpp
     ${PROJECT_SOURCE_DIR}/src/parser.cpp
     ${PROJECT_SOURCE_DIR}/src/nonterminal.cpp
     ${PROJECT_SOURCE_DIR}/src/method.cpp
     ${PROJECT_SOURCE_DIR}/src/reader.cpp
)
    
set (docscript_h
     ${PROJECT_SOURCE_DIR}/include/docc/symbol.h
     ${PROJECT_SOURCE_DIR}/include/docc/token.h
     ${PROJECT_SOURCE_DIR}/include/docc/tokenizer.h
     ${PROJECT_SOURCE_DIR}/include/docc/sanitizer.h
     ${PROJECT_SOURCE_DIR}/include/docc/utils.h
     ${PROJECT_SOURCE_DIR}/include/docc/context.h
     ${PROJECT_SOURCE_DIR}/include/docc/parser.h
     ${PROJECT_SOURCE_DIR}/include/docc/nonterminal.h
     ${PROJECT_SOURCE_DIR}/include/docc/method.h
     ${PROJECT_SOURCE_DIR}/include/docc/reader.h
)

set (docscript_tasks
     ${PROJECT_SOURCE_DIR}/include/docc/tasks/ruler.h
     ${PROJECT_SOURCE_DIR}/include/docc/tasks/reader.h
     ${PROJECT_SOURCE_DIR}/src/tasks/ruler.cpp
     ${PROJECT_SOURCE_DIR}/src/tasks/reader.cpp
)

set (docscript_ast
     ${PROJECT_SOURCE_DIR}/include/docc/ast/constant.h
     ${PROJECT_SOURCE_DIR}/src/ast/constant.cpp
     ${PROJECT_SOURCE_DIR}/include/docc/ast/value.h
     ${PROJECT_SOURCE_DIR}/src/ast/value.cpp
     ${PROJECT_SOURCE_DIR}/include/docc/ast/params.h
     ${PROJECT_SOURCE_DIR}/src/ast/params.cpp
     ${PROJECT_SOURCE_DIR}/include/docc/ast/method.h
     ${PROJECT_SOURCE_DIR}/src/ast/method.cpp
     ${PROJECT_SOURCE_DIR}/include/docc/ast/root.h
     ${PROJECT_SOURCE_DIR}/src/ast/root.cpp
)


#source_group("tasks" FILES ${docscript_tasks})
source_group("ast" FILES ${docscript_ast})

set (docscript_lib
  ${docscript_cpp}
  ${docscript_h}
  ${docscript_ast}
)

