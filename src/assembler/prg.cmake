# 
#	reset 
#
set (assembler_prg)

set (assembler_cpp
    
    ${PROJECT_SOURCE_DIR}/src/assembler/main.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/token.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/tokenizer.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/sanitizer.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/reader.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/context.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/symbol.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/parser.cpp
    ${PROJECT_SOURCE_DIR}/src/assembler/nonterminal.cpp
)
    
set (assembler_h
    ${PROJECT_SOURCE_DIR}/src/assembler/token.h
    ${PROJECT_SOURCE_DIR}/src/assembler/tokenizer.h
    ${PROJECT_SOURCE_DIR}/src/assembler/sanitizer.h
    ${PROJECT_SOURCE_DIR}/src/assembler/reader.h
    ${PROJECT_SOURCE_DIR}/src/assembler/context.h
    ${PROJECT_SOURCE_DIR}/src/assembler/symbol.h
    ${PROJECT_SOURCE_DIR}/src/assembler/parser.h
    ${PROJECT_SOURCE_DIR}/src/assembler/nonterminal.h
)

set (assembler_ast
     ${PROJECT_SOURCE_DIR}/src/assembler/ast/root.h
     ${PROJECT_SOURCE_DIR}/src/assembler/ast/root.cpp
)

source_group("ast" FILES ${assembler_ast})

set (assembler_prg
  ${assembler_cpp}
  ${assembler_h}
  ${assembler_ast}
)

