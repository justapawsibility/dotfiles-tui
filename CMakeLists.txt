cmake_minimum_required(VERSION 3.20)
project(dotfiles-mngr VERSION 1.0.0)

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

FetchContent_Declare(cpp_lib_personal
  GIT_REPOSITORY https://github.com/TobiZehPanda/cpp_lib_personal.git
  GIT_TAG main
)
FetchContent_MakeAvailable(cpp_lib_personal)

add_library(dotfiles_lib
  # src/useful.cpp
  src/cli.cpp
)
target_include_directories(dotfiles_lib PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(dotfiles_lib PUBLIC
    ftxui::component
    ftxui::dom
    ftxui::screen
    argparse
    cpp_lib_personal
)
# include_directories(include)

add_executable(dotfiles
  src/tui.cpp
  )

target_link_libraries(dotfiles PRIVATE
    dotfiles_lib
)

