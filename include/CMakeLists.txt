
if(NOT (PROJ_FOUND AND GEOS_FOUND AND SPARSEHASH_FOUND))
    message(WARNING "Proj+Geos+Sparsehash are needed to check all includes")
    return()
endif()

file(GLOB_RECURSE ALL_HPPS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)

set(GENFILE ${CMAKE_BINARY_DIR}/all_including.cpp)

file(WRITE ${GENFILE} "// generated file with all includes\n")
foreach(hpp ${ALL_HPPS})
    if("${hpp}" MATCHES "^osmium/")
        # GDAL IS SKIPPED FOR NOW
        if(NOT "${hpp}" MATCHES ".*ogr.*")
            string(REGEX REPLACE "\\.hpp" "" hname ${hpp})
            string(REGEX REPLACE "/" "_" longpath ${hname})
            file(APPEND ${GENFILE} "#include <${hpp}>\n")
        endif()
    endif()
endforeach()

file(APPEND ${GENFILE} "int main() { return 0; }\n")
set_source_files_properties(${GENFILE} PROPERTIES GENERATED TRUE)
add_executable(allinclude.exe ${GENFILE})
target_link_libraries(allinclude.exe
    ${EXPAT_LIBRARY} ${OSMPBF_LIBRARIES} ${PROTOBUF_LITE_LIBRARY} ${ZLIB_LIBRARY}
    ${BZIP2_LIBRARIES} ${WIN_LIBS} ${CMAKE_THREAD_LIBS_INIT})

