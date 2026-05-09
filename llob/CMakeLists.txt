# Header-only library for the project (templates, all in headers)
add_library(llob_lib INTERFACE)
target_include_directories(llob_lib INTERFACE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(llob_lib INTERFACE cxx_std_20)
target_link_libraries(llob_lib INTERFACE fmt::fmt)

# Executables
add_executable(main main.cpp)
target_link_libraries(main PRIVATE llob_lib)

add_executable(main2 main2.cpp)
target_link_libraries(main2 PRIVATE llob_lib)

add_executable(main3 main3.cpp)
target_link_libraries(main3 PRIVATE llob_lib)
