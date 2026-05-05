find_package(benchmark CONFIG REQUIRED)

add_executable(llob_bm BenchmarkOrderEngine.cpp)

target_link_libraries(llob_bm PUBLIC 
  llob_lib
  fmt::fmt
  benchmark::benchmark
  benchmark::benchmark_main
)

target_include_directories(llob_bm
  PRIVATE
  ${CMAKE_SOURCE_DIR}/llob
)
