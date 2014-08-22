
if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(
        -Wall
        -Wextra
        -pedantic
        -Wredundant-decls
        -Wdisabled-optimization
        -Wctor-dtor-privacy
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wsign-promo
        -Wold-style-cast
    )

    set(CXX_FLAGS "-O3 -g")
    set(CMAKE_CXX_FLAGS ${CXX_FLAGS})
endif()

add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)


if(SPARSEHASH_FOUND)
    include_directories(${SPARSEHASH_INCLUDE_DIR})
endif()

set(BASIC_LIBS ${EXPAT_LIBRARY} ${OSMPBF_LIBRARIES} ${PROTOBUF_LITE_LIBRARY} ${ZLIB_LIBRARY} ${BZIP2_LIBRARIES} ${WIN_LIBS} ${CMAKE_THREAD_LIBS_INIT})

set(SIMPLE_EXAMPLES
    osmium_read
    osmium_debug
    osmium_convert
    osmium_area_test
    osmium_serdump
)

foreach(example ${SIMPLE_EXAMPLES})
    message(${example})
    add_executable(${example} "${example}.cpp")
    target_link_libraries(${example} ${BASIC_LIBS})
endforeach(example)

unset(Boost_LIBRARIES)
unset(Boost_FOUND)
find_package(Boost 1.38 COMPONENTS program_options)

if(Boost_PROGRAM_OPTIONS_FOUND)
    set(example
        osmium_index
    )
    message(${example})
    add_executable(${example} "${example}.cpp")
    target_link_libraries(${example} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${BASIC_LIBS})
else()
    message("Can not build osmium_index example without Boost program_options")
endif()

set(OGR_EXAMPLES
    osmium_toogr
    osmium_toogr2
)

if(GDAL_FOUND AND GEOS_FOUND AND SPARSEHASH_FOUND)
    include_directories(${GDAL_INCLUDE_DIR})
    include_directories(${GEOS_INCLUDE_DIR})

    foreach(example ${OGR_EXAMPLES})
        message(${example})
        add_executable(${example} "${example}.cpp")
        target_link_libraries(${example} ${GDAL_LIBRARIES} ${GEOS_LIBRARY} ${BASIC_LIBS})
    endforeach(example)
else()
    message("GDAL+GEOS+SparseHash not found, skipping OGR examples")
endif()

