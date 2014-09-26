#----------------------------------------------------------------------
#
#  Libosmium CMakeLists.txt
#
#----------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


#----------------------------------------------------------------------
#
#  Project version
#
#----------------------------------------------------------------------

project(libosmium)

set(LIBOSMIUM_VERSION_MAJOR 0)
set(LIBOSMIUM_VERSION_MINOR 0)
set(LIBOSMIUM_VERSION_PATCH 1)

set(LIBOSMIUM_VERSION ${OSMIUM_VERSION_MAJOR}.${OSMIUM_VERSION_MINOR}.${OSMIUM_VERSION_PATCH})


#----------------------------------------------------------------------
#
#  Build options
#
#  (Change with -DOPTION=VALUE on cmake command line.)
#
#----------------------------------------------------------------------

option(BUILD_EXAMPLES "compile example programs" ON)
option(BUILD_TESTING "compile tests, please run them with ctest" ON)
option(BUILD_TRY_HPPS "compile every hpp" ON)


#----------------------------------------------------------------------
#
#  Find external dependencies
#
#----------------------------------------------------------------------

include(CheckIncludeFiles)
check_include_files(winsock2.h HAVE_WINSOCK)
if(HAVE_WINSOCK)
    add_definitions(-DD_HAVE_WINSOCK)
    list(APPEND WIN_LIBS ws2_32)
endif()

if(CMAKE_COMPILER_IS_GNUCC AND WIN32)
    list(APPEND WIN_LIBS iconv)
endif()

if(MSVC)
    add_definitions(-wd4996 -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
else()
    add_definitions(-std=c++11)
endif()

# check that the essential libraries were found
if(BUILD_EXAMPLES OR BUILD_TESTING OR BUILD_TRY_HPPS)

    find_package(Threads)

    find_package(Boost 1.38)
    mark_as_advanced(CLEAR BOOST_ROOT)

    if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS})
    else()
        set(BOOST_ROOT "NOT FOUND: please choose" CACHE PATH "")
        message(FATAL_ERROR "PLEASE, specify the directory where the Boost library is installed in BOOST_ROOT")
    endif()

    find_package(ZLIB)
    if(ZLIB_FOUND)
        include_directories(${ZLIB_INCLUDE_DIR})
    else()
        mark_as_advanced(CLEAR ZLIB_LIBRARY ZLIB_INCLUDE_DIR)
        message(FATAL_ERROR "Please, specify the ZLIB includes directory and the library file")
    endif()

    find_package(BZip2)
    if(BZIP2_FOUND)
        include_directories(${BZIP2_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "Please, specify the BZip2 includes directory and the library file")
    endif()

    find_package(EXPAT)
    if(EXPAT_FOUND)
        include_directories(${EXPAT_INCLUDE_DIR})
    else()
        mark_as_advanced(CLEAR EXPAT_LIBRARY EXPAT_INCLUDE_DIR)
        message(FATAL_ERROR "Please, specify the EXPAT includes directory and the library file")
    endif()

    find_package(Protobuf)
    if(PROTOBUF_INCLUDE_DIR AND PROTOBUF_LITE_LIBRARY)
        include_directories(${PROTOBUF_INCLUDE_DIR})
    else()
        mark_as_advanced(CLEAR PROTOBUF_INCLUDE_DIR PROTOBUF_LITE_LIBRARY)
        message(FATAL_ERROR "Please, specify the Google PROTOBUF library dir")
    endif()

    find_package(OSMPBF)
    if(OSMPBF_FOUND)
        include_directories(${OSMPBF_INCLUDE_DIRS})
    else()
        message(WARNING "OSMPBF library not found!\n")
    endif()

    ##### Find Google SparseHash
    find_path(SPARSEHASH_INCLUDE_DIR google/sparsetable)
    if(SPARSEHASH_INCLUDE_DIR)
        message(STATUS "SparseHash library found")
        set(SPARSEHASH_FOUND 1)
        include_directories(${SPARSEHASH_INCLUDE_DIR})
    else()
        message(STATUS "SparseHash library not found")
    endif()

    ##### Find GEOS Library
    find_path(GEOS_INCLUDE_DIR geos/geom.h)
    find_library(GEOS_LIBRARY NAMES geos)
    if(GEOS_INCLUDE_DIR AND GEOS_LIBRARY)
        message(STATUS "GEOS library found")
        SET(GEOS_FOUND 1)
        include_directories(${GEOS_INCLUDE_DIR})
    else()
        message(STATUS "GEOS library not found")
    endif()

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

    find_package(GDAL)
    if(GDAL_FOUND)
        include_directories(${GDAL_INCLUDE_DIR})
        set(GDAL_LIBRARIES ${GDAL_LIBRARY} ${ZLIB_LIBRARY} ${WIN_LIBS})
    endif()

    if(MSVC)
        find_path(GETOPT_INCLUDE_DIR getopt.h)
        find_library(GETOPT_LIBRARY NAMES wingetopt)
        if(GETOPT_INCLUDE_DIR AND GETOPT_LIBRARY)
            include_directories(${GETOPT_INCLUDE_DIR})
            list(APPEND WIN_LIBS ${GETOPT_LIBRARY})
        endif()
    endif()
endif()

if(WIN32)
    add_definitions(-DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32 -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0600)
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
    add_subdirectory(test/osm-testdata)
endif()

if(BUILD_TRY_HPPS)
    add_subdirectory(include)
endif()

install(DIRECTORY include/osmium DESTINATION include)
install(FILES include/boost_unicode_iterator.hpp DESTINATION include)

include(CPack)

