#
#
# CMake compatibility issues: don't modify this, please!
cmake_minimum_required (VERSION 3.10)


#
#	                                            
#	     **                                     
#	      **                                    
#	      **                                    
#	      **                                    
#	      **      ****                          
#	  *** **     * ***  *    ****       ****    
#	 *********  *   ****    * ***  *   * ***  * 
#	**   ****  **    **    *   ****   *   ****  
#	**    **   **    **   **         **         
#	**    **   **    **   **         **         
#	**    **   **    **   **         **         
#	**    **   **    **   **         **         
#	**    **    ******    ***     *  ***     *  
#	 *****       ****      *******    *******   
#	  ***                   *****      *****    
#	                                            

#                                                      
# get timestamp and build a patch level from the year and
# the day of the year. This is a unique number to distinguish
# different builds
string(TIMESTAMP THIS_YEAR "%Y")
# Patch level as day of the year
string(TIMESTAMP TWEAK_LEVEL "%j")	# day of the year

#
# set project name/properties
#
project(DOCC 
	VERSION 0.1.${THIS_YEAR}.${TWEAK_LEVEL}
	LANGUAGES CXX C
	DESCRIPTION "Document Generator")
#	HOMEPAGE_URL "https://solostec.ch")


set (${PROJECT_NAME}_COPYRIGHT_YEAR ${THIS_YEAR})


message(STATUS "**") 
message(STATUS "     **                                     ") 
message(STATUS "      **                                    ") 
message(STATUS "      **                                    ") 
message(STATUS "      **                                    ") 
message(STATUS "      **      ****                          ") 
message(STATUS "  *** **     * ***  *    ****       ****    ") 
message(STATUS " *********  *   ****    * ***  *   * ***  * ") 
message(STATUS "**   ****  **    **    *   ****   *   ****  ") 
message(STATUS "**    **   **    **   **         **         ") 
message(STATUS "**    **   **    **   **         **         ") 
message(STATUS "**    **   **    **   **         **         ") 
message(STATUS "**    **   **    **   **         **         ") 
message(STATUS "**    **    ******    ***     *  ***     *  ") 
message(STATUS " *****       ****      *******    *******   ") 
message(STATUS "  ***                   *****      *****    ") 
message(STATUS "**")                                        


#
# Collect and dump some basic information
#
string(TIMESTAMP NOW_UTC "%Y-%m-%dT%H:%M:%SZ")
set (${PROJECT_NAME}_NOW_UTC ${NOW_UTC})

include(ProcessorCount)
ProcessorCount(${PROJECT_NAME}_CPU_COUNT)

if (${${PROJECT_NAME}_CPU_COUNT} LESS 4)
	set(${PROJECT_NAME}_POOL_SIZE 4)
else()
	set(${PROJECT_NAME}_POOL_SIZE ${${PROJECT_NAME}_CPU_COUNT})
endif()


message(STATUS "** CMake              : v${CMAKE_VERSION}")
message(STATUS "** Generator          : ${CMAKE_GENERATOR}")
message(STATUS "** Platform           : ${CMAKE_SYSTEM}")
message(STATUS "** Compiler           : ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "** Timestamp          : ${${PROJECT_NAME}_NOW_UTC}")
message(STATUS "** CPU Cores          : ${${PROJECT_NAME}_CPU_COUNT}")
message(STATUS "** Pool Size          : ${${PROJECT_NAME}_POOL_SIZE}")
message(STATUS "** Patchlevel         : ${PROJECT_VERSION_PATCH}")
message(STATUS "** Tweaklevel         : ${PROJECT_VERSION_TWEAK}")
message(STATUS "** PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "** PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "** ${PROJECT_NAME}_SOURCE_DIR    : ${${PROJECT_NAME}_SOURCE_DIR}")
message(STATUS "** ${PROJECT_NAME}_BINARY_DIR    : ${${PROJECT_NAME}_BINARY_DIR}")

if(UNIX)
# has no meaning on VS
# set default cmake build type to RelWithDebInfo (None Debug Release RelWithDebInfo MinSizeRel)
if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "RelWithDebInfo" )
	message(STATUS "** Set build type  : ${CMAKE_BUILD_TYPE}")
endif()
message(STATUS "** Build type      : ${CMAKE_BUILD_TYPE}")
endif(UNIX)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "** Address Model      : 64 bit")
	set(${PROJECT_NAME}_ADDRESS_MODEL 64)
	set(${PROJECT_NAME}_PREFERRED_BUFFER_SIZE 8192)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(STATUS "** Address Model      : 32 bit")
	set(${PROJECT_NAME}_ADDRESS_MODEL 32)
	set(${PROJECT_NAME}_PREFERRED_BUFFER_SIZE 4096)
else()
	message(STATUS "** Address Model      : not supported")
	set(${PROJECT_NAME}_ADDRESS_MODEL 16)
	set(${PROJECT_NAME}_PREFERRED_BUFFER_SIZE 2048)
endif()

#
# manage unit test: NODE_BUILD_TEST
# default is OFF
#
if(NOT DEFINED ${PROJECT_NAME}_BUILD_TEST)
	set(${PROJECT_NAME}_BUILD_TEST OFF CACHE BOOL "build unit test")
endif()

#
#	setup cyng library
#   -DCYNG_ROOT:path=...
#	assume parallel installation - that is both projects share the same parent directory.
#
if(NOT CYNG_ROOT)
    set(CYNG_ROOT "${PROJECT_SOURCE_DIR}/../cyng" CACHE PATH "CYNG_ROOT")
	message(STATUS "** Set CYNG_ROOT: ${CYNG_ROOT}")
endif()

get_filename_component(CYNG_INCLUDE_DIR "${CYNG_ROOT}/src/main/include" REALPATH)
include_directories("${CYNG_INCLUDE_DIR}")
message(STATUS "** CYNG include path  : ${CYNG_INCLUDE_DIR}")

#
#	setup cyng build directory
#   -DCYNG_BUILD:path=...
#	assume parallel installation - that is both projects share the same parent directory.
#
if (NOT CYNG_BUILD)
    set(CYNG_BUILD "${CYNG_ROOT}/build" CACHE PATH "CYNG_BUILD")
	message(STATUS "** Set CYNG_BUILD: ${CYNG_BUILD}")
endif()

get_filename_component(CYNG_LIBRARY_DIR ${CYNG_BUILD} REALPATH)
include_directories(${CYNG_LIBRARY_DIR})
message(STATUS "** CYNG include path  : ${CYNG_LIBRARY_DIR}")

if (UNIX)
    link_directories(${CYNG_LIBRARY_DIR})
    message(STATUS "** CYNG Libraries     : ${CYNG_LIBRARY_DIR}")
else()
	#
	#	$(ConfigurationName) is a variable used by the MS build system
	#
	link_directories(${CYNG_LIBRARY_DIR}/$(ConfigurationName))
    message(STATUS "** CYNG Libraries     : ${CYNG_LIBRARY_DIR}")
endif()

#
#	Use CMake modules from CYNG project
#
get_filename_component(cyng_MODULE_PATH "${CYNG_ROOT}/src/modules/" REALPATH)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${cyng_MODULE_PATH}")
message(STATUS "** CMake modules      : ${CMAKE_MODULE_PATH}")

#
#	setup crypto library
#   -DCRYPT_ROOT:path=...
#	assume parallel installation - that is both projects share the same parent directory.
#
if(NOT CRYPT_ROOT)
    set(CRYPT_ROOT "${PROJECT_SOURCE_DIR}/../crypto" CACHE PATH "CRYPT_ROOT")
	message(STATUS "** Set CRYPT_ROOT: ${CRYPT_ROOT}")
endif()

get_filename_component(CRYPT_INCLUDE_DIR "${CRYPT_ROOT}/include" REALPATH)
include_directories("${CRYPT_INCLUDE_DIR}")
message(STATUS "** CRYPT include path : ${CRYPT_INCLUDE_DIR}")

#
#	setup cyng build directory
#   -DCRYPT_BUILD:path=...
#	assume parallel installation - that is both projects share the same parent directory.
#
if (NOT CRYPT_BUILD)
    set(CRYPT_BUILD "${CRYPT_ROOT}/build" CACHE PATH "CRYPT_BUILD")
	message(STATUS "** Set CRYPT_BUILD: ${CRYPT_BUILD}")
endif()

get_filename_component(CRYPT_LIBRARY_DIR ${CRYPT_BUILD} REALPATH)
include_directories(${CRYPT_LIBRARY_DIR})
message(STATUS "** CRYPT include path : ${CRYPT_LIBRARY_DIR}")

if (UNIX)
    link_directories(${CRYPT_LIBRARY_DIR})
    message(STATUS "** CRYPT Libraries    : ${CRYPT_LIBRARY_DIR}")
else()
	#
	#	$(ConfigurationName) is a variable used by the MS build system
	#
	link_directories(${CRYPT_LIBRARY_DIR}/$(ConfigurationName))
    message(STATUS "** CRYPT Libraries    : ${CRYPT_LIBRARY_DIR}")
endif()

#
#	setup node library
#   -DNODE_ROOT:path=...
#	assume parallel installation - that is both projects share the same parent directory.
#
if(NOT NODE_ROOT)
    set(NODE_ROOT "${PROJECT_SOURCE_DIR}/../node" CACHE PATH "NODE_ROOT")
	message(STATUS "** Set NODE_ROOT: ${NODE_ROOT}")
endif()

get_filename_component(NODE_INCLUDE_DIR "${NODE_ROOT}/src/main/include" REALPATH)
include_directories("${NODE_INCLUDE_DIR}")
message(STATUS "** NODE include path  : ${NODE_INCLUDE_DIR}")

get_filename_component(NODE_SOURCE_DIR "${NODE_ROOT}/nodes/shared" REALPATH)

#
#	setup node build directory
#   -DNODE_BUILD:path=...
#	assume parallel installation - that is both projects share the same parent directory.
#
if (NOT NODE_BUILD)
    set(NODE_BUILD "${NODE_ROOT}/build" CACHE PATH "NODE_BUILD")
	message(STATUS "** Set NODE_BUILD: ${NODE_BUILD}")
endif()

get_filename_component(NODE_LIBRARY_DIR ${NODE_BUILD} REALPATH)
include_directories(${NODE_LIBRARY_DIR})
message(STATUS "** NODE include path  : ${NODE_LIBRARY_DIR}")

if (UNIX)
    link_directories(${NODE_LIBRARY_DIR})
    message(STATUS "** NODE Libraries     : ${NODE_LIBRARY_DIR}")
else()
	#
	#	$(ConfigurationName) is a variable used by the MS build system
	#
	link_directories(${NODE_LIBRARY_DIR}/$(ConfigurationName))
    message(STATUS "** NODE Libraries     : ${NODE_LIBRARY_DIR}")
endif()

#
#	setup compiler features
#
include(CPPFeatureSupport)

#
# libpugixml-dev (v1.9)
#
# Included as 3party software
#
set(PUGIXML_INCLUDE_DIR "${CYNG_ROOT}/3party/pugixml-190")
message(STATUS "** PugiXML Include   : ${PUGIXML_INCLUDE_DIR}")
add_definitions(-D${PROJECT_NAME}_PUGIXML_INSTALLED)
include_directories(${PUGIXML_INCLUDE_DIR})

#
#	setup C++ compiler
#
if (CMAKE_COMPILER_IS_GNUCXX)

	#
	# gnu C++ select dialect
	#
	
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS  "4.7")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
		message(STATUS "** C++ support     : C++11")
	elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS  "5.1")
	# default since  GCC 6.1
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14") 
		message(STATUS "** C++ support     : C++14")
	elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS  "8.1")
    # partly since 5.0
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17") 
		message(STATUS "** C++ support     : C++17")
	else()
    # partly since 8.0
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a") 
		message(STATUS "** C++ support     : C++2a")
	endif()
	
	# only shared libraries
	set(GLOBAL_LIBRARY_TYPE SHARED)
	
elseif(MSVC)

	#
	# Microsoft C++
	#
	
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "18.0")
	# 	prior Visual Studio 2013
		message(FATAL_ERROR "Insufficient MSVC version")
	endif()

	# only static libraries
	set(GLOBAL_LIBRARY_TYPE STATIC)

	#	This is Windows 7 (and rules out windows vista)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /bigobj /std:c++latest /MP")	
	add_definitions(-D_WIN32_WINNT=0x0601 -D_SCL_SECURE_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
		
else()

	message( FATAL_ERROR "Unknown or missing compiler: ${CMAKE_CXX_COMPILER_ID}" )
	
endif()

#
#	setup Boost library
#
include(SelectBoostLibrary)

#
#	setup SSL library
#
find_package(OpenSSL)
if(OPENSSL_FOUND)

	# satisfy definitions for NODE header files
   add_definitions(-DNODE_SSL_INSTALLED)

    message(STATUS "** openSSL Version       : ${OPENSSL_VERSION}")
    message(STATUS "** openSSL Include       : ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "** openSSL crypto library: ${OPENSSL_CRYPTO_LIBRARY}")
    message(STATUS "** openSSL SSL library   : ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "** openSSL Libraries     : ${OPENSSL_LIBRARIES}")
    
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_LIBRARIES})
        
endif()

#
# include directories for all C/C++ projects
#
include_directories(src/main/include)

#
# configure a header file to pass some of the CMake settings
# to the source code
#
configure_file (
  "${PROJECT_SOURCE_DIR}/src/main/templates/project_info.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_project_info.h"
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#
#	html library
#
include (lib/html/lib.cmake)
add_library(doc_html ${GLOBAL_LIBRARY_TYPE} ${html_lib})

#
#	docscript library
#
include (lib/docscript/lib.cmake)
add_library(docscript_core ${GLOBAL_LIBRARY_TYPE} ${docscript_lib})
target_link_libraries(docscript_core
	cyng_core
	cyng_async
	cyng_crypto
	cyng_vm
	cyng_json
	cyng_csv
	cyng_xml
	smf_protocol_sml)
install(TARGETS docscript_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
#
# docscript compiler
#
include (tools/docc/prg.cmake)
add_executable(docc ${docc})
set(docc_link_libs cyng_io cyng_vm cyng_async cyng_crypto cyng_log cyng_crypto cyng_json cyng_parser cyng_xml crypto docscript_core doc_html ${OPENSSL_LIBRARIES})
if (${GLOBAL_LIBRARY_TYPE} STREQUAL "SHARED")
    if (UNIX)
        list(APPEND docc_link_libs pthread "${Boost_LIBRARIES}" cyng_table)
    endif()
endif()
target_link_libraries(docc ${docc_link_libs})
install(TARGETS docc
    RUNTIME
    DESTINATION bin)


#
# docscript batch
#
include (tools/batch/prg.cmake)
add_executable(batch ${batch})
set(batch_link_libs cyng_core cyng_io cyng_vm cyng_async cyng_crypto cyng_log cyng_parser cyng_xml cyng_table docscript_core crypto doc_html ${OPENSSL_LIBRARIES})
if (${GLOBAL_LIBRARY_TYPE} STREQUAL "SHARED")
    if (UNIX)
        list(APPEND batch_link_libs pthread "${Boost_LIBRARIES}")
    endif()
endif()
target_link_libraries(batch ${batch_link_libs})
install(TARGETS batch
    RUNTIME
    DESTINATION bin)

#
# plog server
#
include (tools/plog/prg.cmake)
add_executable(plog ${plog})
set(plog_link_libs cyng_io cyng_vm cyng_async cyng_crypto cyng_log cyng_parser cyng_xml cyng_mail cyng_table cyng_store cyng_db docscript_core doc_html crypto smf_http_srv smf_https_srv ${OPENSSL_LIBRARIES})
if (${GLOBAL_LIBRARY_TYPE} STREQUAL "SHARED")
    if (UNIX)
		list(APPEND plog_link_libs pthread "${Boost_LIBRARIES}")
    endif()
endif()
target_link_libraries(plog ${plog_link_libs})
install(TARGETS plog
    RUNTIME
    DESTINATION bin)

#
#   Generate configuration files
#   Place service files in /etc/systemd/system/
#
#   some usefull commands:
#   systemctl list-unit-files --state=enabled
#   sudo systemctl daemon-reload
#   journalctl -f -u node-http
#
if(UNIX)

	# docc
	configure_file (
		  "${PROJECT_SOURCE_DIR}/tools/docc/templates/docc.linux.cgf.in"
		  "${PROJECT_BINARY_DIR}/docc_v${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.cfg")
		  
	# plog
	configure_file (
		  "${PROJECT_SOURCE_DIR}/tools/plog/templates/plog.linux.cgf.in"
		  "${PROJECT_BINARY_DIR}/plog_v${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.cfg")
		  
  else()
	  configure_file (
			"${PROJECT_SOURCE_DIR}/tools/docc/templates/docc.linux.cgf.in"
			"${PROJECT_BINARY_DIR}/docc_v${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.cfg")
			
	  configure_file (
			"${PROJECT_SOURCE_DIR}/tools/plog/templates/plog.linux.cgf.in"
			"${PROJECT_BINARY_DIR}/plog_v${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.cfg")
  endif()
#
# CPack configuration
#
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} - Programmable document generator")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "info@solostec.ch")
set(CPACK_PACKAGE_VENDOR "solosTec")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_PACKAGE_DESCRIPTION "CYNG library")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

if(UNIX)
#	set(CPACK_PACKAGING_INSTALL_PREFIX "/opt")
	set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
	set(CPACK_GENERATOR "RPM")
elseif(WIN32)

	set(CPACK_GENERATOR "NSIS64")
	set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/logo.ico)
	set(CPACK_NSIS_MUI_UNIICON ${PROJECT_SOURCE_DIR}/logo.ico)
	#set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\vcredist_x86.exe\\\" /q:a'")
	set(CPACK_NSIS_MODIFY_PATH ON)
	set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.solostec.com")

else()
	set(CPACK_GENERATOR "DEB")
endif()


include(CPack)
