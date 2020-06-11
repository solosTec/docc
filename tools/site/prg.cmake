# 
#	reset 
#
set (site)

set (site_cpp
	tools/site/src/main.cpp
	tools/site/src/site.cpp
	tools/src/driver.cpp
	tools/src/reader.cpp
)
    
set (site_h
	 "${PROJECT_BINARY_DIR}/DOCC_project_info.h"
	tools/site/src/site.h
	tools/src/driver.h
	tools/src/reader.h
)

# define the site program
set (site
  ${site_cpp}
  ${site_h}
)
