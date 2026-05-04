add_executable(main main.cpp)
add_executable(main2 main2.cpp)

target_link_libraries(main PUBLIC fmt::fmt)
target_link_libraries(main2 PUBLIC fmt::fmt)

target_include_directories(main
  PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_include_directories(main2
  PRIVATE
  ${CMAKE_SOURCE_DIR}
)
