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
    ${PROJECT_SOURCE_DIR}/include/asm/token.h
    ${PROJECT_SOURCE_DIR}/include/asm/tokenizer.h
    ${PROJECT_SOURCE_DIR}/include/asm/sanitizer.h
    ${PROJECT_SOURCE_DIR}/include/asm/reader.h
    ${PROJECT_SOURCE_DIR}/include/asm/context.h
    ${PROJECT_SOURCE_DIR}/include/asm/symbol.h
    ${PROJECT_SOURCE_DIR}/include/asm/parser.h
    ${PROJECT_SOURCE_DIR}/include/asm/nonterminal.h
)


set (docasm_ast
     ${PROJECT_SOURCE_DIR}/include/asm/ast/root.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/label.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/literal.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/operation.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/value.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/push.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/invoke.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/forward.h
     ${PROJECT_SOURCE_DIR}/include/asm/ast/jump.h
     ${PROJECT_SOURCE_DIR}/src/ast/root.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/label.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/literal.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/operation.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/value.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/push.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/invoke.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/forward.cpp
     ${PROJECT_SOURCE_DIR}/src/ast/jump.cpp
)


source_group("ast" FILES ${docasm_ast})

set (docasm_lib
  ${docasm_cpp}
  ${docasm_h}
  ${docasm_ast}
)

