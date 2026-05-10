#include <cstdio>
#include <filesystem>
#include <iostream>
#include "cli.hpp"
#include <glok/main.hpp>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <argparse/argparse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>

using namespace std;
using namespace ftxui;

// ─── Helpers ────────────────────────────────────────────────────────────────

bool compareConfig(Config* a, Config* b) {
    return a->name < b->name;
}

void sort_configs(vector<Config*>& configs) {
    sort(configs.begin(), configs.end(), compareConfig);
}

pair<vector<string>, vector<string>> list_config_tui(vector<Config*>& configs) {
    vector<string> installed, not_installed;
    for (auto& config : configs) {
        path temp_dest = replace_home(config->dest);
        if (is_symlink(temp_dest))
            installed.push_back(config->name);
        else
            not_installed.push_back(config->name);
    }
    return {installed, not_installed};
}

// ─── Config IO ──────────────────────────────────────────────────────────────

void load_config(vector<Config*>& configurations) {
    string buffer;
    ifstream file(CONF_FILE);
    while (getline(file, buffer)) {
        vector<string> parts = split(buffer, ",");
        Config* temp = nullptr;
        if (parts.size() == 3) {
            temp = new Config(parts[0], parts[1], parts[2]);
        } else if (parts.size() == 5) {
            temp = new Config5(parts[0], parts[1], parts[2], parts[3], parts[4]);
        } else {
            cout << "invalid config: " << buffer << endl;
            continue;
        }
        configurations.push_back(temp);
    }
}

void write_config(vector<Config*>& c) {
    ofstream file(CONF_FILE);
    sort_configs(c);
    for (const auto& config : c) {
        Config5* c5 = dynamic_cast<Config5*>(config);
        if (c5) {
            file << c5->name << "," << c5->source.string() << "," << c5->dest.string()
                 << "," << c5->source2.string() << "," << c5->dest2.string() << endl;
        } else {
            file << config->name << "," << config->source.string() << "," << config->dest.string() << endl;
        }
    }
}

void list_config(vector<Config*>& configs) {
    auto [installed, not_installed] = list_config_tui(configs);
    cout << "--------INSTALLED-------" << endl;
    for (const auto& i : installed)
        cout << i << endl;
    cout << "--------NOT INSTALLED--------" << endl;
    for (const auto& n : not_installed)
        cout << n << endl;
}

pair<vector<string>, vector<string>> sudo_bind(vector<Config*>& configs) {
    vector<string> sudo_list, nonsudo;
    for (const auto& config : configs) {
        if (config->name.find('*') != string::npos)
            sudo_list.push_back(config->name);
        else
            nonsudo.push_back(config->name);
    }
    return {nonsudo, sudo_list};
}

// ─── TUI ────────────────────────────────────────────────────────────────────

Component VMenu(vector<string>* entries, int* selected, vector<Config*>* configs) {
    auto option = MenuOption::Vertical();
    option.entries_option.transform = [configs](EntryState state) {
        Element e = state.active ? text("[" + state.label + "]")
                                 : text(" " + state.label + " ");
        auto [installed, not_installed] = list_config_tui(*configs);
        if (find(installed.begin(), installed.end(), state.label) != installed.end())
            e = e | color(Color::Green);
        if (state.focused || state.active)
            e = e | bold;
        return e;
    };
    return Menu(entries, selected, option);
}

// ─── Main ───────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    bool force = false;
    vector<Config*> configs;
    load_config(configs);
    auto [nonsudo, sudo_list] = sudo_bind(configs);

    argparse::ArgumentParser program("dotfiles");
    program.add_argument("-l", "--list").flag().help("lists configurations");
    program.add_argument("-s", "--sort").flag().help("sorts and saves configurations");
    program.add_group("Management");
    program.add_argument("-f", "--force").flag().help("enables the use of force");
    program.add_argument("-i", "--install")
        .nargs(argparse::nargs_pattern::at_least_one)
        .metavar("CONFIG").help("installs a config");
    program.add_argument("-r", "--remove")
        .nargs(argparse::nargs_pattern::at_least_one)
        .metavar("CONFIG").help("uninstalls a config");
    program.add_argument("-d", "--delete")
        .nargs(argparse::nargs_pattern::at_least_one)
        .metavar("CONFIG").help("deletes a config (dangerous)");

    try {
        program.parse_args(argc, argv);
    } catch (const exception& err) {
        cerr << err.what() << endl;
        cerr << program;
        exit(1);
    }

    if (argc >= 2) {
        if (program["-f"] == true)
          force = true;
        if (program["-s"] == true)
            write_config(configs);

        if (program.present("-i")) {
            for (const auto& str : program.get<vector<string>>("-i"))
                for (auto& config : configs)
                    if (config->name == str)
                        config->install(force);
        }

        if (program.present("-r")) {
            for (const auto& str : program.get<vector<string>>("-r"))
                for (auto& config : configs)
                    if (config->name == str)
                        config->remove();
        }

        if (program["-l"] == true) {
            list_config(configs);
            exit(0);
        }

        return 0;
    }

    // ── TUI mode ──
    auto screen = ScreenInteractive::Fullscreen();

    auto guide = Renderer([&] {
        Elements elems = {
            text(" q: quit ")              | bold | color(Color::RedLight),
            text(" │ "),
            text(" i/space: install ")     | bold | color(Color::GreenLight),
            text(" │ "),
            text(" f: force install ")     | bold | color(Color::YellowLight),
            text(" │ "),
            text(" u: uninstall ")         | bold | color(Color::BlueLight),
        };
        return hbox(elems) | border | center;
    });

    int selected = 0;
    int tab_selected = 0;
    vector<string> tab_values{"Normal", "Sudo"};

    auto tab_toggle     = Toggle(&tab_values, &tab_selected);
    auto vmenu_nonsudo  = VMenu(&nonsudo,   &selected, &configs);
    auto vmenu_sudo     = VMenu(&sudo_list, &selected, &configs);

    Components tab_children = {vmenu_nonsudo, vmenu_sudo};
    auto tab_container = Container::Tab(tab_children, &tab_selected);

    Components global_children = {tab_toggle, tab_container};
    auto global = Container::Vertical(global_children);

    auto renderer = Renderer(global, [&] {
        Elements elems = {
            text(" Dotfiles Manager ") | bold | center | color(Color::White),
            separator(),
            tab_toggle->Render()     | border | center,
            separator(),
            tab_container->Render()  | flex,
            separator(),
            guide->Render(),
        };
        return vbox(elems) | borderDouble | size(WIDTH, LESS_THAN, 100) | flex;
    });

    renderer |= CatchEvent([&](Event event) {
        if (event == Event::Character('q') && tab_selected != 2) {
            screen.ExitLoopClosure()();
            return true;
        }
        if (event == Event::Character('u') && tab_selected == 0 && selected < (int)nonsudo.size()) {
            for (auto& config : configs)
                if (config->name == nonsudo[selected])
                    config->remove();
            return true;
        }
        if (event == Event::Character('f') && tab_selected == 0 && selected < (int)nonsudo.size()) {
            for (auto& config : configs)
                if (config->name == nonsudo[selected])
                    config->install(true);
            return true;
        }
        if ((event == Event::Character(' ') || event == Event::Character('i') || event == Event::Return)
            && tab_selected == 0 && selected < (int)nonsudo.size()) {
            for (auto& config : configs)
                if (config->name == nonsudo[selected])
                    config->install(false);
            return true;
        }
        return false;
    });

    screen.Loop(renderer);
    return 0;
}
