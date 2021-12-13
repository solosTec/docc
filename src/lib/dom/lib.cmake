# 
#	reset 
#
set (dom_lib)

set (dom_cpp
     ${PROJECT_SOURCE_DIR}/src/dom.cpp
     ${PROJECT_SOURCE_DIR}/src/formatting.cpp
)
    
set (dom_h
     ${PROJECT_SOURCE_DIR}/include/html/dom.hpp
     ${PROJECT_SOURCE_DIR}/include/html/formatting.h
)

set (dom_code
     ${PROJECT_SOURCE_DIR}/src/code.cpp
     ${PROJECT_SOURCE_DIR}/src/code_json.cpp
     ${PROJECT_SOURCE_DIR}/src/code_curly.cpp
     ${PROJECT_SOURCE_DIR}/src/code_docscript.cpp
     ${PROJECT_SOURCE_DIR}/src/code_binary.cpp
     ${PROJECT_SOURCE_DIR}/src/code_generic.cpp
     ${PROJECT_SOURCE_DIR}/src/code_html.cpp
     ${PROJECT_SOURCE_DIR}/src/code_cmake.cpp
     ${PROJECT_SOURCE_DIR}/include/html/code.h
     ${PROJECT_SOURCE_DIR}/include/html/code_json.h
     ${PROJECT_SOURCE_DIR}/include/html/code_curly.h
     ${PROJECT_SOURCE_DIR}/include/html/code_docscript.h
     ${PROJECT_SOURCE_DIR}/include/html/code_binary.h
     ${PROJECT_SOURCE_DIR}/include/html/code_generic.h
     ${PROJECT_SOURCE_DIR}/include/html/code_html.h
     ${PROJECT_SOURCE_DIR}/include/html/code_cmake.h
)

source_group("formatter" FILES ${dom_code})

set (dom_lib
    ${dom_cpp}
    ${dom_h}
    ${dom_code}
)

