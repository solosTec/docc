# Dependencies
find_package(Boost 1.75 REQUIRED COMPONENTS unit_test_framework)

add_executable(unit_test 
    main.cpp
    src/test_download.cpp
    src/root_certificates.hpp)

target_compile_features(unit_test PRIVATE cxx_std_20)

target_compile_definitions(unit_test
    PRIVATE 
        BOOST_TEST_DYN_LINK 
        BOOST_ASIO_HAS_MOVE 
        $<$<CONFIG:Debug>:_DEBUG_TEST>
        NO_BLOCKING
        "$<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>"
)

#
# -Wno-psabi for gcc > 7.1
# add_compile_options(/utf-8)
#
target_compile_options(unit_test 
    PRIVATE 
        $<$<AND:$<VERSION_GREATER_EQUAL:${CMAKE_CXX_COMPILER_VERSION},7.1>,$<CXX_COMPILER_ID:GNU>>:-Wno-psabi>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

target_include_directories(unit_test
    PRIVATE
        $<INSTALL_INTERFACE:include>
#        ${URI_INCLUDE_DIR}
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        ${CYNG_INCLUDE_DIRS}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${OPENSSL_INCLUDE_DIR}>
)
message(STATUS "** Boost_UNIT_TEST_FRAMEWORK_LIBRARY      : ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_DEBUG}")

target_link_directories(unit_test
    PRIVATE
        Boost::url
        ${Boost_LIBRARY_DIRS}
)

#
#   Linking against the SQLite C library requires "-ldl"
#
target_link_libraries(unit_test 
    PRIVATE
        Boost::url
        ${CYNG_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        "$<$<PLATFORM_ID:Linux>:pthread;${CMAKE_DL_LIBS}>"
        "$<$<PLATFORM_ID:Windows>:odbc32>"
        Boost::locale
        Boost::unit_test_framework
        $<$<AND:$<VERSION_LESS_EQUAL:${CMAKE_CXX_COMPILER_VERSION},9.1>,$<CXX_COMPILER_ID:GNU>>:stdc++fs>        
)

# include(CTest)
enable_testing()

add_test(DemoTest unit_test)

