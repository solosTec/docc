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

if(UNIX)
	set (docc_resources 
		tools/docc/templates/docc.linux.cgf.in
	)
else()
	set (docc_resources 
		tools/docc/templates/docc.windows.cgf.in
	)
endif()
source_group("resources" FILES ${docc_resources})


# define the docc program
set (docc
  ${docc_cpp}
  ${docc_h}
  ${docc_resources}
)
