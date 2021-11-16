#
#
# CMake compatibility issues: don't modify this, please!
cmake_minimum_required (VERSION 3.16)


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
# set project name/properties
#
project(docc 
	VERSION 0.2
	LANGUAGES CXX C
	DESCRIPTION "Document Generator"
	HOMEPAGE_URL "https://solostec.ch")

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
find_package(Boost 1.76
	REQUIRED COMPONENTS filesystem program_options system)
if(Boost_FOUND)

    message(STATUS "** Boost Version         : ${Boost_VERSION}")
    message(STATUS "** Boost libaries        : ${Boost_LIBRARIES}")

	link_directories(${Boost_LIBRARY_DIRS})

else()

	message(ERROR "Boost library not found")

endif(Boost_FOUND)

#
#	setup SSL library
#
find_package(OpenSSL)
if(OPENSSL_FOUND)

    message(STATUS "** openSSL Version       : ${OPENSSL_VERSION}")
    message(STATUS "** openSSL Include       : ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "** openSSL crypto library: ${OPENSSL_CRYPTO_LIBRARY}")
    message(STATUS "** openSSL SSL library   : ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "** openSSL Libraries     : ${OPENSSL_LIBRARIES}")
         
else()

	message(ERROR "OpenSSL library not found")
	
endif()

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
# if(${CMAKE_VERSION} VERSION_LESS 3.14)
#   include(cmake/add_FetchContent_MakeAvailable.cmake)
# endif()

# Formatting library
# Use <format> when C++20 is available
#
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 8.0.1)
FetchContent_MakeAvailable(fmtlib)
# Adds fmt::fmt

# XML library
#
FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml
  GIT_TAG v1.11.4)
FetchContent_MakeAvailable(pugixml)
# Adds pugixml

# chrono library
#
FetchContent_Declare(
  date_src 
  GIT_REPOSITORY https://github.com/HowardHinnant/date.git
  GIT_TAG v3.0.1)
FetchContent_MakeAvailable(date_src)
# Adds date 

#
#	generate a version.hpp file
#
configure_file(src/version.hpp.in "${PROJECT_BINARY_DIR}/include/version.hpp")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(CYNG REQUIRED)
if (CYNG_FOUND)
    message(STATUS "** CYNG_INCLUDE_DIRS     : ${CYNG_INCLUDE_DIRS}")
    message(STATUS "** CYNG_LIBRARIES        : ${CYNG_LIBRARIES}")
    message(STATUS "** cyng_db               : ${cyng_db}")
    message(STATUS "** cyng_io               : ${cyng_io}")
    message(STATUS "** cyng_log              : ${cyng_log}")
    message(STATUS "** cyng_obj              : ${cyng_obj}")
    message(STATUS "** cyng_store            : ${cyng_store}")
    message(STATUS "** cyng_sys              : ${cyng_sys}")
    message(STATUS "** cyng_task             : ${cyng_task}")
    message(STATUS "** cyng_vm               : ${cyng_vm}")
    message(STATUS "** cyng_sqlite3          : ${cyng_sqlite3}")
else()
    message(FATAL_ERROR "** CYNG library not found")
endif()

#
#	libraries
#
if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/src/lib)
	add_subdirectory(src/lib/compiler)
	add_subdirectory(src/lib/assembler)
	add_subdirectory(src/lib/runtime)
	add_subdirectory(src/lib/dom)
endif()

#
# docScript compiler
#

add_executable(docc 
    ${CMAKE_SOURCE_DIR}/src/compiler/main.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/controller.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/controller.h
)

target_include_directories(docc
	PUBLIC
		${PROJECT_BINARY_DIR}/include
		${CMAKE_SOURCE_DIR}/src/compiler
 		${OPENSSL_INCLUDE_DIR}
 		${Boost_INCLUDE_DIRS}
)

target_compile_features(docc PRIVATE cxx_std_20)

target_compile_options(docc 
	PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-pedantic>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W2>"
)

target_compile_definitions(docc
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

target_link_libraries(docc 
	PUBLIC 
		docscript
        fmt::fmt
#		CYNG::CYNG
		${CYNG_LIBRARIES}
		${OPENSSL_LIBRARIES}
		"$<$<PLATFORM_ID:Linux>:pthread;${Boost_LIBRARIES}>"
)

install(TARGETS docc
    RUNTIME
    DESTINATION bin)

#
# docScript assembler
#
include (src/assembler/prg.cmake)

add_executable(doca ${assembler_prg})

target_include_directories(doca
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/assembler
        ${CYNG_INCLUDE_DIRS}
		${PROJECT_BINARY_DIR}/include
 		${OPENSSL_INCLUDE_DIR}
 		${Boost_INCLUDE_DIRS}
)

target_compile_features(doca PRIVATE cxx_std_20)

target_compile_options(doca 
    PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-pedantic>"
    PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W2>"
)

target_compile_definitions(doca
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

target_link_libraries(doca 
	PUBLIC 
		docasm
		docscript
        fmt::fmt
		${CYNG_LIBRARIES}
		${OPENSSL_LIBRARIES}
		"$<$<PLATFORM_ID:Linux>:pthread;${Boost_LIBRARIES}>"
)

install(TARGETS doca
    RUNTIME
    DESTINATION bin)

#
# generic docScript runtime
#
include (src/runtime/prg.cmake)

add_executable(docrt ${runtime_prg})

target_include_directories(docrt
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/runtime
        ${CYNG_INCLUDE_DIRS}
		${PROJECT_BINARY_DIR}/include
 		${OPENSSL_INCLUDE_DIR}
 		${Boost_INCLUDE_DIRS}
)

target_compile_features(docrt PRIVATE cxx_std_20)

target_compile_options(docrt 
    PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-pedantic>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W2>"
)

target_compile_definitions(docrt
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

target_link_libraries(docrt 
	PUBLIC 
		docruntime
		docscript
        fmt::fmt
		${CYNG_LIBRARIES}
		${OPENSSL_LIBRARIES}
		"$<$<PLATFORM_ID:Linux>:pthread;${Boost_LIBRARIES}>"
)

install(TARGETS docrt
    RUNTIME
    DESTINATION bin)

#
# HTML doc2html generator
#
include (src/runtime/html/prg.cmake)

add_executable(doc2html ${doc2html_prg})

target_include_directories(doc2html
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/runtime
        ${CYNG_INCLUDE_DIRS}
		${PROJECT_BINARY_DIR}/include
 		${OPENSSL_INCLUDE_DIR}
 		${Boost_INCLUDE_DIRS}
)

target_compile_features(doc2html PRIVATE cxx_std_20)

target_compile_options(doc2html 
    PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-pedantic>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W2>"
)

target_compile_definitions(doc2html
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

target_link_libraries(doc2html 
	PUBLIC 
		docscript
		docasm
		docruntime
		dom
        fmt::fmt
		${CYNG_LIBRARIES}
		${OPENSSL_LIBRARIES}
		"$<$<PLATFORM_ID:Linux>:pthread;${Boost_LIBRARIES}>"
)

install(TARGETS doc2html
    RUNTIME
    DESTINATION bin)

#
# LaTeX doc2LaTeX generator
#
include (src/runtime/LaTeX/prg.cmake)

add_executable(doc2LaTeX ${doc2LaTeX_prg})

target_include_directories(doc2LaTeX
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/runtime
        ${CYNG_INCLUDE_DIRS}
		${PROJECT_BINARY_DIR}/include
 		${OPENSSL_INCLUDE_DIR}
 		${Boost_INCLUDE_DIRS}
)

target_compile_features(doc2LaTeX PRIVATE cxx_std_20)

target_compile_options(doc2LaTeX 
    PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-pedantic>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W2>"
)

target_compile_definitions(doc2LaTeX
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

target_link_libraries(doc2LaTeX 
	PRIVATE 
		docscript
		docruntime
        fmt::fmt
		${CYNG_LIBRARIES}
		${OPENSSL_LIBRARIES}
		"$<$<PLATFORM_ID:Linux>:pthread;${Boost_LIBRARIES}>"
)

install(TARGETS doc2LaTeX
    RUNTIME
    DESTINATION bin)

#
# markdown doc2md generator
#
include (src/runtime/markdown/prg.cmake)

add_executable(doc2md ${doc2md_prg})

target_include_directories(doc2md
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/runtime
        ${CYNG_INCLUDE_DIRS}
		${PROJECT_BINARY_DIR}/include
 		${OPENSSL_INCLUDE_DIR}
 		${Boost_INCLUDE_DIRS}
)

target_compile_features(doc2md PRIVATE cxx_std_20)

target_compile_options(doc2md 
    PRIVATE "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-pedantic>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W2>"
)

target_compile_definitions(doc2md
	PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")

target_link_libraries(doc2md 
	PUBLIC 
		docscript
		docruntime
        fmt::fmt
		${CYNG_LIBRARIES}
		${OPENSSL_LIBRARIES}
		"$<$<PLATFORM_ID:Linux>:pthread;${Boost_LIBRARIES}>"
)

install(TARGETS doc2md
    RUNTIME DESTINATION bin)

#
# batch files
#
if(WIN32)
    configure_file (
		    "${CMAKE_CURRENT_SOURCE_DIR}/src/build.cmd.in"
		    "${PROJECT_BINARY_DIR}/build.cmd")

endif(WIN32)


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
set(CPACK_RPM_PACKAGE_DESCRIPTION "DOCC library")
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
