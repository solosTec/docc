# 
#	reset 
#
set (docc)

set (docc_cpp
	tools/docc/src/main.cpp
	tools/src/driver.cpp
	tools/src/reader.cpp
)
    
set (docc_h
	tools/src/driver.h
	tools/src/reader.h
)

# define the docc program
set (docc
  ${docc_cpp}
  ${docc_h}
)
