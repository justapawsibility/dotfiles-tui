cmake_minimum_required(VERSION 4.0.0)
project(MyProject VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

FetchContent_Declare(argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
)
FetchContent_MakeAvailable(argparse)

include_directories(include)

add_executable(a.out
  main.cpp
  include/useful.cpp
  include/useful.hpp)

target_link_libraries(a.out PRIVATE
    # Chose a submodule
    ftxui::component
    ftxui::dom
    ftxui::screen
    argparse
)

