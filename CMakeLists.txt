cmake_minimum_required(VERSION 4.0.0)
project(dotfiles_manager)

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

FetchContent_Declare(tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_Declare(glok
  GIT_REPOSITORY https://github.com/TobiZehPanda/grand_library_of_knowledge.git
  GIT_TAG main
)
FetchContent_MakeAvailable(glok)

add_executable(dotfiles
  src/main.cpp
)

target_include_directories(dotfiles PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(dotfiles PUBLIC
    ftxui::component
    ftxui::dom
    ftxui::screen
    argparse
    tomlplusplus::tomlplusplus
    glok
)
# include_directories(include)


# target_link_libraries(dotfiles PRIVATE
#     dotfiles_lib
# )

