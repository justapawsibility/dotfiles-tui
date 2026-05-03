cmake_minimum_required(VERSION 4.0.0)
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

add_library(dotfiles_lib include/cli.cpp
  include/useful.cpp
)
target_include_directories(dotfiles_lib PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)
# include_directories(include)

add_executable(dotfiles
  tui.cpp
  )

target_link_libraries(dotfiles PRIVATE
    # Chose a submodule
    ftxui::component
    ftxui::dom
    ftxui::screen
    argparse
    dotfiles_lib
)

