add_executable(llob_bm BenchmarkOrderEngine.cpp)

target_link_libraries(llob_bm PUBLIC 
  llob_lib
  fmt::fmt
)

target_include_directories(llob_bm
  PRIVATE
  ${CMAKE_SOURCE_DIR}/llob
)
