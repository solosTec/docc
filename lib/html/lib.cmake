# 
#	reset 
#
set (html_lib)

set (html_cpp
	lib/html/src/element.cpp
	lib/html/src/attribute.cpp
	lib/html/src/node.cpp
)
    
set (html_h
	src/main/include/html/element.hpp
	src/main/include/html/attribute.hpp
	src/main/include/html/node.hpp
)


# define the HTML lib
set (html_lib
  ${html_cpp}
  ${html_h}
)

