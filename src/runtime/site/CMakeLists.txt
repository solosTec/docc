#
#
# CMake compatibility issues: don't modify this, please!
cmake_minimum_required (VERSION 3.16)


#
#	
#	                                             
#	      *******                                
#	    *       ***    *         *               
#	   *         **   ***       **               
#	   **        *     *        **               
#	    ***                   ********           
#	   ** ***        ***     ********     ***    
#	    *** ***       ***       **       * ***   
#	      *** ***      **       **      *   ***  
#	        *** ***    **       **     **    *** 
#	          ** ***   **       **     ********  
#	           ** **   **       **     *******   
#	            * *    **       **     **        
#	  ***        *     **       **     ****    * 
#	 *  *********      *** *     **     *******  
#	*     *****         ***              *****   
#	*                                            
#	 **                                          
#	                                            

#
# set project name/properties
#
project(docSite 
	VERSION 0.2
	LANGUAGES CXX C
	DESCRIPTION "Site Generator"
	HOMEPAGE_URL "https://solostec.ch")

message(STATUS "") 
message(STATUS "      *******                                ") 
message(STATUS "    *       ***    *         *               ") 
message(STATUS "   *         **   ***       **               ") 
message(STATUS "   **        *     *        **               ") 
message(STATUS "    ***                   ********           ") 
message(STATUS "   ** ***        ***     ********     ***    ") 
message(STATUS "    *** ***       ***       **       * ***   ") 
message(STATUS "      *** ***      **       **      *   ***  ") 
message(STATUS "        *** ***    **       **     **    *** ") 
message(STATUS "          ** ***   **       **     ********  ") 
message(STATUS "           ** **   **       **     *******   ") 
message(STATUS "            * *    **       **     **        ") 
message(STATUS "  ***        *     **       **     ****    * ") 
message(STATUS " *  *********      *** *     **     *******  ") 
message(STATUS "*     *****         ***              *****   ") 
message(STATUS "*                                            ")                                              
                                             

string(TIMESTAMP CURRENT_YEAR "%Y")

message(STATUS "** CMake              : v${CMAKE_VERSION}")
message(STATUS "** Generator          : ${CMAKE_GENERATOR}")
message(STATUS "** Platform           : ${CMAKE_SYSTEM}")
message(STATUS "** Compiler           : ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "** Version            : ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
message(STATUS "** PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "** PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "** CMAKE_SOURCE_DIR   : ${CMAKE_SOURCE_DIR}")
message(STATUS "** CMAKE_BINARY_DIR   : ${CMAKE_BINARY_DIR}")
message(STATUS "** CURRENT_SOURCE_DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "** ${PROJECT_NAME}_SOURCE_DIR : ${${PROJECT_NAME}_SOURCE_DIR}")
message(STATUS "** ${PROJECT_NAME}_BINARY_DIR : ${${PROJECT_NAME}_BINARY_DIR}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
#	boost library
#
if(Boost_FOUND)

    message(STATUS "** Boost Version      : ${Boost_VERSION}")
    message(STATUS "** Boost libaries     : ${Boost_LIBRARIES}")

else()

	message(ERROR "Boost library not found")

endif(Boost_FOUND)

#
#	setup SSL library
#
if(OPENSSL_FOUND)

    message(STATUS "** openSSL Version    : ${OPENSSL_VERSION}")
    message(STATUS "** openSSL Include    : ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "** openSSL crypto lib : ${OPENSSL_CRYPTO_LIBRARY}")
    message(STATUS "** openSSL SSL lib    : ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "** openSSL Libraries  : ${OPENSSL_LIBRARIES}")
         
else()

	message(ERROR "OpenSSL library not found")
	
endif()


if (CYNG_FOUND)
    message(STATUS "** CYNG_INCLUDE_DIRS  : ${CYNG_INCLUDE_DIRS}")
    message(STATUS "** CYNG_LIBRARIES     : ${CYNG_LIBRARIES}")
    message(STATUS "** cyng_db            : ${cyng_db}")
    message(STATUS "** cyng_io            : ${cyng_io}")
    message(STATUS "** cyng_log           : ${cyng_log}")
    message(STATUS "** cyng_obj           : ${cyng_obj}")
    message(STATUS "** cyng_store         : ${cyng_store}")
    message(STATUS "** cyng_sys           : ${cyng_sys}")
    message(STATUS "** cyng_task          : ${cyng_task}")
    message(STATUS "** cyng_vm            : ${cyng_vm}")
    message(STATUS "** cyng_sqlite3       : ${cyng_sqlite3}")
else()
    message(FATAL_ERROR "** CYNG library not found")
endif()

if (SMFSEC_FOUND)
    message(STATUS "** SMFSEC_INCLUDE_DIRS: ${SMFSEC_INCLUDE_DIRS}")
    message(STATUS "** SMFSEC_LIBRARIES   : ${SMFSEC_LIBRARIES}")
else()
    message(FATAL_ERROR "** SMFSEC library not found")
endif()


#
# docSite generator
#
include (prg.cmake)
add_executable(doc2site ${doc2site_prg})

target_include_directories(doc2site
	PUBLIC
		${CMAKE_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
 		${OPENSSL_INCLUDE_DIR}
 		${Boost_INCLUDE_DIRS}
)

target_compile_features(doc2site PRIVATE cxx_std_20)

target_compile_options(doc2site 
	PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-pedantic>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W2>"
)

target_compile_definitions(doc2site
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

target_link_libraries(doc2site 
	PUBLIC 
		docscript
        fmt::fmt
		${CYNG_LIBRARIES}
		${OPENSSL_LIBRARIES}
		"$<$<PLATFORM_ID:Linux>:pthread;${Boost_LIBRARIES}>"
)

install(TARGETS doc2site
    RUNTIME
    DESTINATION bin)

#
# constants
#
configure_file (
		"${CMAKE_CURRENT_SOURCE_DIR}/site_defs.h.in"
		"${CMAKE_BINARY_DIR}/include/site_defs.h")

