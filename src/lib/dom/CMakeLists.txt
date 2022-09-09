#
#
# CMake compatibility issues: don't modify this, please!
cmake_minimum_required (VERSION 3.16)

#
# set project name/properties
#
project(docScript 
	VERSION 0.2
	LANGUAGES CXX
	DESCRIPTION "DocDom Library"
	HOMEPAGE_URL "https://solostec.ch")


message(STATUS "  **")                                                                                                                            
message(STATUS "       **                                ")
message(STATUS "        **                               ")
message(STATUS "        **                               ")
message(STATUS "        **                               ")
message(STATUS "        **      ****                     ")
message(STATUS "    *** **     * ***  * *** **** ****    ")
message(STATUS "   *********  *   ****   *** **** ***  * ")
message(STATUS "  **   ****  **    **     **  **** ****  ")
message(STATUS "  **    **   **    **     **   **   **   ")
message(STATUS "  **    **   **    **     **   **   **   ")
message(STATUS "  **    **   **    **     **   **   **   ")
message(STATUS "  **    **   **    **     **   **   **   ")
message(STATUS "  **    **    ******      **   **   **   ")
message(STATUS "   *****       ****       ***  ***  ***  ")
message(STATUS "    ***                    ***  ***  *** ")
message(STATUS "  **")                                   
                                   

string(TIMESTAMP CURRENT_YEAR "%Y")

message(STATUS "** CMake                    : v${CMAKE_VERSION}")
message(STATUS "** Generator                : ${CMAKE_GENERATOR}")
message(STATUS "** Platform                 : ${CMAKE_SYSTEM}")
message(STATUS "** Compiler                 : ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "** Version                  : ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
message(STATUS "** PROJECT_SOURCE_DIR       : ${PROJECT_SOURCE_DIR}")
message(STATUS "** PROJECT_BINARY_DIR       : ${PROJECT_BINARY_DIR}")
message(STATUS "** ${PROJECT_NAME}_SOURCE_DIR     : ${${PROJECT_NAME}_SOURCE_DIR}")
message(STATUS "** ${PROJECT_NAME}_BINARY_DIR     : ${${PROJECT_NAME}_BINARY_DIR}")
message(STATUS "** CMAKE_SOURCE_DIR         : ${CMAKE_SOURCE_DIR}")
message(STATUS "** CMAKE_CURRENT_SOURCE_DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "** CYNG_INCLUDE_DIRS        : ${CYNG_INCLUDE_DIRS}")
message(STATUS "** CYNG_LIBRARIES           : ${CYNG_LIBRARIES}")
message(STATUS "** Boost_LIBRARIES          : ${Boost_LIBRARIES}")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
	set(GLOBAL_LIBRARY_TYPE STATIC)
else()
	set(GLOBAL_LIBRARY_TYPE SHARED)
endif()

include (lib.cmake)
add_library(dom ${GLOBAL_LIBRARY_TYPE} ${dom_lib})
add_library(Docc::dom ALIAS dom)

target_include_directories(dom
    INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CYNG_INCLUDE_DIRS}
 		${Boost_INCLUDE_DIRS}
)

target_link_libraries(dom 
	PRIVATE
		cyng::cyng
		"$<$<PLATFORM_ID:Linux>:pthread;Boost::boost>"
)


target_compile_definitions(dom
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

install(TARGETS dom
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
    

