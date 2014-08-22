set(PACKAGE OSMIUM)
set(PACKAGE_NAME OSMIUM)

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(OSMIUM)

option(BUILD_EXAMPLES "compile example programs" ON)
option(BUILD_TESTING "compile tests, please run them with ctest" ON)
option(BUILD_TRY_HPPS "compile every hpp" ON)


find_package(Boost 1.38)
mark_as_advanced(CLEAR BOOST_ROOT)


include(CheckIncludeFiles)
check_include_files(winsock2.h HAVE_WINSOCK)
if(HAVE_WINSOCK)
    add_definitions(-DD_HAVE_WINSOCK)
    list(APPEND WIN_LIBS ws2_32)
endif()

if(CMAKE_COMPILER_IS_GNUCC AND WIN32)
    list(APPEND WIN_LIBS iconv)
endif()

if(NOT CMAKE_COMPILER_IS_GNUCC AND WIN32)
    find_path(GETOPT_INCLUDE_DIR getopt.h)
    find_library(GETOPT_LIBRARY NAMES wingetopt)
    add_definitions(-wd4996 -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
    if(GETOPT_INCLUDE_DIR AND GETOPT_LIBRARY)
        include_directories(${GETOPT_INCLUDE_DIR})
        list(APPEND WIN_LIBS ${GETOPT_LIBRARY})
    endif()
else()
    add_definitions(-std=c++11)
endif()

find_package(ZLIB)
find_package(BZip2)
find_package(EXPAT)
find_package(Protobuf)
find_package(Threads)

##### Find OSMPBF Library
find_path(OSMPBF_INCLUDE_DIR osmpbf/osmpbf.h)
find_library(OSMPBF_LIBRARY NAMES osmpbf)
#####


# check that the essential libraries were found
if(BUILD_EXAMPLES OR BUILD_TESTING OR BUILD_TESTING OR BUILD_OSMJS)

    if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS})
    else()
        set(BOOST_ROOT "NOT FOUND: please choose" CACHE PATH "")
        message(FATAL_ERROR "PLEASE, specify the directory where the Boost library is installed in BOOST_ROOT")
    endif()

    if(ZLIB_FOUND)
        include_directories(${ZLIB_INCLUDE_DIR})
    else()
        mark_as_advanced(CLEAR ZLIB_LIBRARY ZLIB_INCLUDE_DIR)
        message(FATAL_ERROR "Please, specify the ZLIB includes directory and the library file")
    endif()

    if(BZIP2_FOUND)
        include_directories(${BZIP2_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "Please, specify the BZip2 includes directory and the library file")
    endif()

    if(EXPAT_FOUND)
        include_directories(${EXPAT_INCLUDE_DIR})
    else()
        mark_as_advanced(CLEAR EXPAT_LIBRARY EXPAT_INCLUDE_DIR)
        message(FATAL_ERROR "Please, specify the EXPAT includes directory and the library file")
    endif()

    if(PROTOBUF_INCLUDE_DIR AND PROTOBUF_LITE_LIBRARY)
        include_directories(${PROTOBUF_INCLUDE_DIR})
    else()
        mark_as_advanced(CLEAR PROTOBUF_INCLUDE_DIR PROTOBUF_LITE_LIBRARY)
        message(FATAL_ERROR "Please, specify the Google PROTOBUF library dir")
    endif()

    if(OSMPBF_LIBRARY AND OSMPBF_INCLUDE_DIR)
        include_directories(${OSMPBF_INCLUDE_DIR})
        set(OSMPBF_FOUND 1)
        set(OSMPBF_LIBRARIES ${OSMPBF_LIBRARY} ${PROTOBUF_LITE_LIBRARY} ${ZLIB_LIBRARY})
        message(STATUS "OSMPBF library found")
    else()
        message(FATAL_ERROR "OSMPBF library not found, please specify its location manually")
    endif()

endif()

find_package(LibXml2)

##### Find Google SparseHash
find_path(SPARSEHASH_INCLUDE_DIR google/sparsetable)
if(SPARSEHASH_INCLUDE_DIR)
    message(STATUS "SparseHash library found")
    set(SPARSEHASH_FOUND 1)
else()
    message(STATUS "SparseHash library not found")
endif()
#####


##### Find GEOS Library
find_path(GEOS_INCLUDE_DIR geos/geom.h)
find_library(GEOS_LIBRARY NAMES geos)
if(GEOS_INCLUDE_DIR AND GEOS_LIBRARY)
    message(STATUS "GEOS library found")
    SET(GEOS_FOUND 1)
else()
    message(STATUS "GEOS library not found")
endif()
#####

##### Find Proj.4 Library
find_path(PROJ_INCLUDE_DIR proj_api.h)
find_library(PROJ_LIBRARY NAMES proj)
if(PROJ_INCLUDE_DIR AND PROJ_LIBRARY)
    message(STATUS "PROJ library found")
    set(PROJ_FOUND 1)
    include_directories(${PROJ_INCLUDE_DIR})
else()
    message(STATUS "PROJ library not found")
endif()
#####



find_package(GDAL)
if(GDAL_FOUND)
    set(GDAL_LIBRARIES ${GDAL_LIBRARY} ${ZLIB_LIBRARY} ${WIN_LIBS})
endif()

if(WIN32)
    add_definitions(-DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32 -DWINVER=0x0500  -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0600)
    set(CPACK_GENERATOR ZIP)
else(WIN32)
    set(CPACK_GENERATOR TGZ)
endif(WIN32)

include_directories(BEFORE include)

enable_testing()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


if(BUILD_TESTING)
    add_subdirectory(test)
endif()

if(BUILD_TRY_HPPS)
    add_subdirectory(include)
endif()

install(DIRECTORY include/osmium DESTINATION include)
install(FILES include/boost_unicode_iterator.hpp DESTINATION include)

include(CPack)

