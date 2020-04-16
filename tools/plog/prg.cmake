# 
#	reset 
#
set (plog)

set (plog_cpp
	tools/plog/src/main.cpp
	tools/plog/src/controller.cpp
	tools/plog/src/logic.cpp
# from node project
	${NODE_SOURCE_DIR}/db/db_schemes.cpp
)
    
set (plog_h
	"${PROJECT_BINARY_DIR}/DOCC_project_info.h"
	tools/plog/src/controller.h
	tools/plog/src/logic.h
)

if(UNIX)
	set (plog_resources 
		tools/plog/templates/plog.linux.cgf.in
	)
else()
	set (plog_resources 
		tools/plog/templates/plog.windows.cgf.in
	)
endif()
source_group("resources" FILES ${plog_resources})

# define the plog program
set (plog
  ${plog_cpp}
  ${plog_h}
  ${plog_resources}
)
