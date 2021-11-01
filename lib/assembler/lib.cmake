# 
#	reset 
#
set (docasm_lib)

set (docasm_cpp
    ${PROJECT_SOURCE_DIR}/src/token.cpp
    ${PROJECT_SOURCE_DIR}/src/tokenizer.cpp
    ${PROJECT_SOURCE_DIR}/src/sanitizer.cpp
    ${PROJECT_SOURCE_DIR}/src/reader.cpp
    ${PROJECT_SOURCE_DIR}/src/context.cpp
    ${PROJECT_SOURCE_DIR}/src/symbol.cpp
    ${PROJECT_SOURCE_DIR}/src/parser.cpp
    ${PROJECT_SOURCE_DIR}/src/nonterminal.cpp
)
    
set (docasm_h
    ${PROJECT_SOURCE_DIR}/include/token.h
    ${PROJECT_SOURCE_DIR}/include/tokenizer.h
    ${PROJECT_SOURCE_DIR}/include/sanitizer.h
    ${PROJECT_SOURCE_DIR}/include/reader.h
    ${PROJECT_SOURCE_DIR}/include/context.h
    ${PROJECT_SOURCE_DIR}/include/symbol.h
    ${PROJECT_SOURCE_DIR}/include/parser.h
    ${PROJECT_SOURCE_DIR}/include/nonterminal.h
)


set (docasm_ast
     ${PROJECT_SOURCE_DIR}/include/ast/root.h
     ${PROJECT_SOURCE_DIR}/include/ast/label.h
     ${PROJECT_SOURCE_DIR}/include/ast/literal.h
     ${PROJECT_SOURCE_DIR}/include/ast/operation.h
     ${PROJECT_SOURCE_DIR}/include/ast/value.h
     ${PROJECT_SOURCE_DIR}/include/ast/push.h
     ${PROJECT_SOURCE_DIR}/include/ast/invoke.h
     ${PROJECT_SOURCE_DIR}/src/ast/root.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/label.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/literal.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/operation.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/value.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/push.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/invoke.cpp
)


source_group("ast" FILES ${docasm_ast})

set (docasm_lib
  ${docasm_cpp}
  ${docasm_h}
  ${docasm_ast}
)

