# 
#	reset 
#
set (html_lib)

set (html_cpp
	lib/html/src/dom.cpp
)
    
set (html_h
	src/main/include/html/dom.hpp
)


# define the HTML lib
set (html_lib
  ${html_cpp}
  ${html_h}
)

