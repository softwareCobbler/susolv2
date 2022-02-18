cmake_minimum_required(VERSION 3.14)
project(susolv)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(susolv
  include/susolv/cellIndexLookup.h
  include/susolv/board.h
  src/susolv.cpp
  src/board.cpp
)
target_include_directories(susolv PRIVATE ./include)


enable_testing()

add_executable(
    hello_test
    test/hello_test.cpp
    include/susolv/board.h
    src/board.cpp
)

target_include_directories(hello_test PRIVATE ./include)

target_link_libraries(
    hello_test
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)