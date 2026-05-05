find_package(GTest CONFIG REQUIRED)

add_executable(llob_tests TestPriceLevel.cpp TestOrderBook.cpp)

target_link_libraries(llob_tests PUBLIC 
  llob_lib
  fmt::fmt
  GTest::gtest
  GTest::gtest_main
)

target_include_directories(llob_tests
  PRIVATE
  ${CMAKE_SOURCE_DIR}/llob
)
