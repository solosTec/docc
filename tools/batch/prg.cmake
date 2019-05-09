# 
#	reset 
#
set (batch)

set (batch_cpp
	tools/batch/src/main.cpp
)
    
set (batch_h
	 "${PROJECT_BINARY_DIR}/DOCC_project_info.h"
)

# define the batch program
set (batch
  ${batch_cpp}
  ${batch_h}
)
