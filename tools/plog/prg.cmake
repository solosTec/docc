# 
#	reset 
#
set (plog)

set (plog_cpp
	tools/plog/src/main.cpp
)
    
set (plog_h
	 "${PROJECT_BINARY_DIR}/DOCC_project_info.h"
)

# define the plog program
set (plog
  ${plog_cpp}
  ${plog_h}
)
