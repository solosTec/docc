# 
#	reset 
#
set (docscript_lib)

set (docscript_cpp
	lib/docscript/src/sanitizer.cpp
	lib/docscript/src/token.cpp  
	lib/docscript/src/pre_compiler.cpp  
	lib/docscript/src/tokenizer.cpp  
	lib/docscript/src/symbol.cpp  
	lib/docscript/src/node.cpp  
	lib/docscript/src/ast.cpp  
	lib/docscript/src/parser.cpp  
	lib/docscript/src/lookup.cpp  
#	lib/docscript/src/generator.cpp  
#	lib/docscript/src/library.cpp  
	lib/docscript/src/statistics.cpp  
)
    
set (docscript_h
	src/main/include/docscript/sanitizer.h
	src/main/include/docscript/token.h  
	src/main/include/docscript/pre_compiler.h  
	src/main/include/docscript/tokenizer.h  
	src/main/include/docscript/symbol.h  
	src/main/include/docscript/node.h
	src/main/include/docscript/ast.h
	src/main/include/docscript/parser.h  
	src/main/include/docscript/include.h  
	src/main/include/docscript/lookup.h  
#	src/main/include/docscript/docscript.h  
#	src/main/include/docscript/generator.h  
#	src/main/include/docscript/library.h  
	src/main/include/docscript/statistics.h  
)

set (docscript_generator
	src/main/include/docscript/generator/generator.h
	src/main/include/docscript/generator/gen_html.h  
	src/main/include/docscript/generator/gen_md.h  
	src/main/include/docscript/generator/gen_LaTeX.h  
	src/main/include/docscript/generator/numbering.h
	lib/docscript/src/generator/generator.cpp
	lib/docscript/src/generator/gen_html.cpp
	lib/docscript/src/generator/gen_md.cpp
	lib/docscript/src/generator/gen_LaTeX.cpp
	lib/docscript/src/generator/numbering.cpp
)

set (docscript_filter
	lib/docscript/src/generator/filter/text_to_html.h
	lib/docscript/src/generator/filter/cpp_to_html.h
	lib/docscript/src/generator/filter/json_to_html.h
	lib/docscript/src/generator/filter/html_to_html.h
	lib/docscript/src/generator/filter/docscript_to_html.h
	lib/docscript/src/generator/filter/binary_to_html.h

	lib/docscript/src/generator/filter/text_to_html.cpp
	lib/docscript/src/generator/filter/cpp_to_html.cpp
	lib/docscript/src/generator/filter/json_to_html.cpp
	lib/docscript/src/generator/filter/html_to_html.cpp
	lib/docscript/src/generator/filter/docscript_to_html.cpp
	lib/docscript/src/generator/filter/binary_to_html.cpp
)

source_group("generator" FILES ${docscript_generator})
source_group("filter" FILES ${docscript_filter})

# define the docscript lib
set (docscript_lib
  ${docscript_cpp}
  ${docscript_h}
  ${docscript_generator}
  ${docscript_filter}
)

if(WIN32)
#	set_source_files_properties(lib/docscript/src/lexer.cpp PROPERTIES COMPILE_FLAGS /utf-8)
	set_source_files_properties(lib/docscript/src/sanitizer.cpp PROPERTIES COMPILE_FLAGS /utf-8)
endif()