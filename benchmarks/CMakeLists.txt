message(STATUS "Configuring benchmarks")

set(BENCHMARKS
    count
    index_map
    CACHE STRING "Benchmark programs"
)

#----------------------------------------------------------------------
#
#  Configure benchmarks
#
#----------------------------------------------------------------------

message(STATUS "Configuring benchmarks - Building these benchmarks:")
foreach(benchmark ${BENCHMARKS})
    message(STATUS "  - osmium_benchmark_${benchmark}")
    add_executable(osmium_benchmark_${benchmark} "osmium_benchmark_${benchmark}.cpp")
    target_link_libraries(osmium_benchmark_${benchmark} ${OSMIUM_IO_LIBRARIES} ${BENCHMARK_LIBS_${benchmark}})
endforeach()

foreach(file setup run_benchmarks)
    configure_file(${file}.sh ${CMAKE_CURRENT_BINARY_DIR}/${file}.sh @ONLY)
endforeach()

foreach(benchmark ${BENCHMARKS})
    configure_file(run_benchmark_${benchmark}.sh ${CMAKE_CURRENT_BINARY_DIR}/run_benchmark_${benchmark}.sh @ONLY)
endforeach()

message(STATUS "Configuring benchmarks done")

