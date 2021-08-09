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
)
    
set (docscript_h
     ${PROJECT_SOURCE_DIR}/include/symbol.h
     ${PROJECT_SOURCE_DIR}/include/token.h
     ${PROJECT_SOURCE_DIR}/include/tokenizer.h
     ${PROJECT_SOURCE_DIR}/include/sanitizer.h
     ${PROJECT_SOURCE_DIR}/include/utils.h
     ${PROJECT_SOURCE_DIR}/include/context.h
     ${PROJECT_SOURCE_DIR}/include/parser.h
)

set (docscript_tasks
     ${PROJECT_SOURCE_DIR}/include/tasks/ruler.h
     ${PROJECT_SOURCE_DIR}/include/tasks/reader.h
     ${PROJECT_SOURCE_DIR}/src/tasks/ruler.cpp
     ${PROJECT_SOURCE_DIR}/src/tasks/reader.cpp
)



source_group("tasks" FILES ${docscript_tasks})

set (docscript_lib
  ${docscript_cpp}
  ${docscript_h}
  ${docscript_tasks}
)

