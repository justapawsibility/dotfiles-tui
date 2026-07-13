# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A single-binary dotfiles manager (`dotfiles`) written in C++. It reads a
`configurations.toml` file describing named config entries, each mapping a
`source` path to a `dest` path, and manages them as **symlinks** (`dest` ->
`source`). It has two modes: a command-line mode (argparse flags) and, when
invoked with no arguments, a fullscreen terminal UI (FTXUI).

## Build & run

Uses CMake presets with the Ninja generator. Two toolchains are defined:

```bash
cmake --preset x86_64          # configure (native)
cmake --build build/x86_64     # build -> build/x86_64/dotfiles

cmake --preset aarch64         # cross-compile (needs aarch64-linux-gnu-g++)
cmake --build build/aarch64
```

- Requires CMake >= 4.0. All dependencies are fetched at configure time via
  `FetchContent` (no system packages needed beyond a compiler + Ninja), so the
  first configure needs network access and is slow.
- There is no test suite, linter config, or CI in this repo.
- `compile_commands.json` is emitted into the build dir (gitignored); symlink it
  to the repo root if your editor/LSP needs it.

### Running

The binary reads/writes `configurations.toml` **from the current working
directory**, so run it from a directory containing that file.

```bash
./build/x86_64/dotfiles              # no args -> launch the TUI
./build/x86_64/dotfiles -l           # list installed / not-installed
./build/x86_64/dotfiles -i NAME...   # install (symlink) config(s)
./build/x86_64/dotfiles -r NAME...   # uninstall (remove symlink)
./build/x86_64/dotfiles -d NAME...   # delete: remove symlink + drop entry from the toml
./build/x86_64/dotfiles -f -i NAME   # --force: remove_all(dest) before linking
./build/x86_64/dotfiles -s           # sort configs and rewrite the toml
```

## Dependencies (all via FetchContent)

- **ftxui** (v6.1.9) — terminal UI (`Menu`, `Toggle`, `Container`, `Renderer`).
- **argparse** — CLI parsing.
- **tomlplusplus** (v3.4.0) — reads/writes `configurations.toml`.
- **glok** (`grand_library_of_knowledge`, `main`) — a personal utility library.
  Provides `replace_home()` (expands `~`/home in paths) and other helpers used
  throughout; it is `#include <glok/main.hpp>` and also pulls in the `std`/`cout`
  usage. When something like `replace_home` or `split` is undefined, it lives here,
  not in this repo.

## Architecture

Only two source files carry the logic:

- **`include/cli.hpp`** — the config model.
  - `Config` (name, source, dest) is the base type. `install(force)` creates the
    symlink (`create_directories` for the parent, `remove_all(dest)` first if
    forced); `remove()` deletes the symlink; `print()` dumps fields.
  - `Config5` derives from `Config` and adds a **second** source/dest pair
    (`source2`/`dest2`), overriding each method to also handle the second link.
    This is the mechanism for a single named entry that manages two symlinks.
    Polymorphism is via `Config*` + `dynamic_cast<Config5*>` (see `write_config`).

- **`src/main.cpp`** — IO, CLI dispatch, and TUI.
  - `load_config` / `write_config` parse and serialize the `[[config]]` array in
    `configurations.toml`. An entry becomes a `Config5` iff both `source2` and
    `dest2` are present; otherwise a `Config`. Entries missing name/source/dest
    are skipped. `Config` objects are heap-allocated with `new` and never freed.
  - `sudo_bind` splits configs into two lists by name: names containing `'*'` go
    to the "Sudo" tab, everything else to "Normal". This `'*'` convention is the
    only thing distinguishing the two TUI tabs.
  - "Installed" is determined dynamically by checking `is_symlink(dest)` — there
    is no persisted install state. `VMenu` colors installed entries green by
    re-checking this on every render.
  - TUI layout: a `Toggle` (Normal/Sudo) over a `Container::Tab` of two `VMenu`s,
    wired through `CatchEvent`. Keys: `q` quit, `i`/`space`/`Enter` install,
    `f` force-install, `u` uninstall — all act on the selected entry of the
    active tab.

## Notes / gotchas

- Commit history is terse (`update`, `moved_shit`); don't rely on it for intent.
- `main.cpp` and `cli.hpp` both do `using namespace std;` / `using namespace
  filesystem;` — match that style; unqualified `path`, `cout`, etc. are expected.
- Old line-based (CSV) config IO is left commented out in `main.cpp`; the TOML
  path is the live one.
