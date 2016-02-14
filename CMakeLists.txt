PROJECT(cornelich C CXX)

CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_COLOR_MAKEFILE ON)

IF((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND CMAKE_BUILD_TYPE STREQUAL "Coverage")
    INCLUDE(CodeCoverage)
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(FindVALGRIND)
INCLUDE(Untar)

# We are using ctest for running some of the tests (e.g. ctest -D ExperimentalMemCheck)
INCLUDE(CTest)
ENABLE_TESTING()

IF(UNIX)
    FIND_PACKAGE(VALGRIND)
    SET(CTEST_MEMORYCHECK_COMMAND, ${VALGRIND_PROGRAM})
    SET(CTEST_MEMORYCHECK_COMMAND_OPTIONS, "--trace-children=yes --leak-check=full")
ENDIF()

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

IF(MSVC)
    ADD_DEFINITIONS( /W4 )
    MESSAGE(WARNING "MSVC support not tested")
ELSE()
    ADD_DEFINITIONS( -D_FILE_OFFSET_BITS=64 )  # Large File Support
    SET(_warnings_cxx_list 
        "-Wall"
        "-Wextra"
        "-Werror"
#        "-pedantic" # Commented out due to a boost pragma
        "-fstrict-aliasing"
        "-Wstrict-aliasing=2"
        "-Wfloat-equal"
        "-Wno-missing-field-initializers"
        "-Wno-missing-braces"
        "-Wshadow"
        "-Wunreachable-code"
        "-Wredundant-decls"
        "-Wpointer-arith"
        "-Wundef"
        "-Wcast-align"
        "-Wswitch-enum"
        #"-fsanitize=thread"
        #"-fsanitize=address"
        #"-fsanitize=memory"
        #"-fsanitize=undefined"
    )
    SET(_perf_cxx_list
        "-mtune=native"
        "-march=native"
        "-fomit-frame-pointer"
    )
    SET(_perf_linker_list
        "-flto" 
        "-fwhole-program"
    )
    IF(ENABLE_PROFILING)
        MESSAGE("${Yellow}    Profiling flags enabled${ColourReset}")
        LIST(REMOVE_ITEM _perf_cxx_list "-fomit-frame-pointer" "-foptimize-sibling-calls" "-finline-functions-called-once" "-finline-functions")
        LIST(APPEND _perf_cxx_list "-g3" "-ggdb3" "-pg" "-fno-inline-functions" "-fno-omit-frame-pointer" "-fno-inline-functions-called-once" "-fno-optimize-sibling-calls")
    ENDIF()
    STRING(REPLACE ";" " " _warnings_cxx "${_warnings_cxx_list}")
    STRING(REPLACE ";" " " _perf_cxx "${_perf_cxx_list}")
    STRING(REPLACE ";" " " _perf_linker "${_perf_linker_list}")
    
    MESSAGE("${Yellow}    Warning flags: ${Green}${_warnings_cxx}${ColourReset}")
    MESSAGE("${Yellow}    Release flags: ${Green}${_perf_cxx}${ColourReset}")
    MESSAGE("${Yellow}    Linker  flags: ${Green}${_perf_linker}${ColourReset}")

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_warnings_cxx}")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++14 ${_warnings_cxx} ${_perf_cxx}")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${_perf_linker}")
ENDIF()

ADD_DEFINITIONS(-DBOOST_SPIRIT_THREADSAFE)

SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost COMPONENTS filesystem system REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

IF(Boost_FOUND)
    MESSAGE("${Yellow}    Boost information")
    MESSAGE("${Yellow}    Boost_INCLUDE_DIRS: ${Green}${Boost_INCLUDE_DIRS}${ColourReset}")
    MESSAGE("${Yellow}    Boost_LIBRARY_DIRS: ${Green}${Boost_LIBRARY_DIRS}${ColourReset}")
    MESSAGE("${Yellow}    Boost Libraries: ${Green}${Boost_LIBRARIES}${ColourReset}")
ENDIF()

IF(NOT Boost_USE_STATIC_LIBS)
    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
ENDIF()

SET(CATCH_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/contrib/catch")

ADD_SUBDIRECTORY(cornelich)
ADD_SUBDIRECTORY(cornelich_test)
ADD_SUBDIRECTORY(cornelich_sample)

# Special entry for creating the coverage make target:
#   make coverage
#   ctest -D ExperimentalCoverage
IF(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    SETUP_TARGET_FOR_COVERAGE(coverage ctest coverage)
ENDIF()
