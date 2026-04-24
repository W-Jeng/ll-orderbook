add_executable(main main.cpp)

target_link_libraries(main PUBLIC fmt::fmt)

target_include_directories(main
  PRIVATE
  ${CMAKE_SOURCE_DIR}
)
