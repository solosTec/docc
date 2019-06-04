# 
#	reset 
#
set (batch)

set (batch_cpp
	tools/batch/src/main.cpp
	tools/batch/src/batch.cpp
	tools/src/driver.cpp
	tools/src/reader.cpp
)
    
set (batch_h
	 "${PROJECT_BINARY_DIR}/DOCC_project_info.h"
	tools/batch/src/batch.h
	tools/src/driver.h
	tools/src/reader.h
)

# define the batch program
set (batch
  ${batch_cpp}
  ${batch_h}
)
