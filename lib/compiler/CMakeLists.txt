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
	DESCRIPTION "DocScript Library"
	HOMEPAGE_URL "https://solostec.ch")

string(TIMESTAMP CURRENT_YEAR "%Y")

message(STATUS "** CMake              : v${CMAKE_VERSION}")
message(STATUS "** Generator          : ${CMAKE_GENERATOR}")
message(STATUS "** Platform           : ${CMAKE_SYSTEM}")
message(STATUS "** Compiler           : ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "** Version            : ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
message(STATUS "** PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "** PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "** ${PROJECT_NAME}_SOURCE_DIR    : ${${PROJECT_NAME}_SOURCE_DIR}")
message(STATUS "** ${PROJECT_NAME}_BINARY_DIR    : ${${PROJECT_NAME}_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
#	boost library
#
#find_package(Boost 1.76
#	REQUIRED COMPONENTS program_options system)
if(NOT Boost_FOUND)

	message(ERROR "Boost library not found")

endif()

#
#	setup SSL library
#
find_package(OpenSSL)
if(NOT OPENSSL_FOUND)

 	message(ERROR "OpenSSL library not found")
          
endif()

#configure_file(src/version.hpp.in "${PROJECT_BINARY_DIR}/version.hpp")

if(MSVC)
	set(GLOBAL_LIBRARY_TYPE STATIC)
else()
	set(GLOBAL_LIBRARY_TYPE SHARED)
endif()

include (lib.cmake)
add_library(docscript ${GLOBAL_LIBRARY_TYPE} ${docscript_lib})

target_include_directories(docscript
    INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
#		${CYNG_INCLUDE_DIRS}
		${OPENSSL_INCLUDE_DIR}
		${Boost_INCLUDE_DIRS}
)

target_link_libraries(docscript 
	CYNG::CYNG
    fmt::fmt
)

target_compile_definitions(docscript
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

install(TARGETS docscript
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
    

