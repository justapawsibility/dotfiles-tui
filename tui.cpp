#include "include/useful.hpp"
#include "include/cli.hpp"
#include <argparse/argparse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>

using namespace ftxui;

struct config_list {
  public:
    vector<string> installed;
    vector<string> not_installed;
};

Component VMenu(std::vector<std::string>* entries, int* selected, vector<config>* c);

config_list list_config_tui(vector<config>* configs) {
  config_list list;
  for (config confi : *configs) {
      path temp_dest = replace_home(confi.dest);
      path temp_dest2 = replace_home(confi.dest2);
      if (is_symlink(temp_dest)) {
        list.installed.push_back(confi.name);
      }
      else {
        list.not_installed.push_back(confi.name);
      }
  }
  return list;
}

void print_tui(vector<config> *configs,string name) {
  print_config(configs, name);
}

int main(int argc, char *argv[]) {
  config_lista temp = load_config();
  vector<string> nonsudo, sudo;
  vector<config> confi;
  confi = temp.configurations;
  nonsudo = temp.nonsudo;
  sudo = temp.sudo;
  bool forced = false;

  argparse::ArgumentParser program("dotfiles");
  program.add_argument("-q", "--quiet")
    .flag()
    .help("quiet mode");
  program.add_argument("-l", "--list")
    .flag()
    .help("lists configurations");
  program.add_argument("-f", "--force")
    .flag()
    .help("enables the use of force");
  program.add_group("Management");
  program.add_argument("-i", "--install")
    .nargs(argparse::nargs_pattern::at_least_one)
    .metavar("CONFIG")
    .help("installs a config");
  program.add_argument("-r", "--remove")
    .nargs(argparse::nargs_pattern::at_least_one)
    .metavar("CONFIG")
    .help("uninstalls a config");
  program.add_group("Creation");
  program.add_argument("-a", "--add")
    .nargs(3)
    .metavar("VAR")
    .help("add a new config");
  program.add_argument("-a5", "--add5")
    .nargs(5)
    .metavar("VAR")
    .help("add a new config");
  program.add_argument("-d", "--delete")
    .nargs(argparse::nargs_pattern::at_least_one)
    .metavar("CONFIG")
    .help("deletes a config (dangerous)");

  try {
    program.parse_args(argc, argv);
  }
  catch (const exception& err) {
    cerr << err.what() << endl;
    cerr << program;
    exit(1);
  }

  if (program["-q"] == true || program["-l"] == true) {
    if (argc == 2) {
      if (program["-q"] == true) {
        cerr << program;
        exit(1);
      }
      if (program["-l"] == true) {
        list_config(&confi);
        exit(0);
      }
    }
    if (program["-f"] == true) {
      forced = true;
    }
    if (program.present("-i")){
      auto parse = program.get<vector<string>>("-i");
      for (string str : parse) {
        install_config(&confi, str, forced);
      }
    }
    if (program.present("-r")){
      auto parse = program.get<vector<string>>("-r");
      for (string str : parse) {
        remove_config(&confi, str);
      }
    }
    if (program.present("-a")){
      auto parse = program.get<vector<string>>("-a");
      add3_config(&confi, parse[0], parse[1], parse[2]);
    }
    if (program.present("-a5")){
      auto parse = program.get<vector<string>>("-a5");
      add5_config(&confi, parse[0], parse[1], parse[2], parse[3], parse[4]);
    }
    if (program.present("-d")){
      string answer;
      auto parse = program.get<vector<string>>("-d");
      if (forced) {
        for (string str : parse) {
          remove_config(&confi, str);
          delete_config(&confi, str);
          cout << str << " was deleted!" << endl;
          confi = load_config().configurations;
        }
      }
      else {
        cout << "Are you sure you want to delete? (y/n) ";
        cin >> answer;
        if (answer == "y" || answer == "Y") {
          for (string str : parse) {
            remove_config(&confi, str);
            delete_config(&confi, str);
            cout << str << " was deleted!" << endl;
            confi = load_config().configurations;
          }
        }
        else {
          cout << "Exiting..." << endl;
          exit(0);
        }
      }
    }
    return 0;
  }
  
  else {
    auto screen = ScreenInteractive::TerminalOutput();
    auto guide = Renderer([&] {
      return text("q = quit, space/enter = install, f = force install, u = uninstall") | border | flex_grow | size(WIDTH, LESS_THAN, 80) | center;
    });
    int selected = 0;
    int tab_selected = 0;
    vector<string> tab_values{
      "normal",
      "sudo",
      "input",
    };
    string name;
    string source;
    string dest;
    string source2;
    string dest2;
    auto tab_toggle = Toggle(&tab_values, &tab_selected);
    auto vmenu_nonsudo = VMenu(&nonsudo, &selected, &confi);
    auto vmenu_sudo = VMenu(&sudo, &selected, &confi);
    auto input_form = Container::Vertical({
        Container::Horizontal({
          Input(&name, "NAME"),
          Input(&source, "SOURCE"),
          Input(&dest, "DEST"),
          Input(&source2, "SOURCE2 - OPTIONAL"),
          Input(&dest2, "DEST2 - OPTIONAL"),
        }),
        Container::Horizontal({
            Button({"Submit - 3", [&] { add3_config(&confi, name, source, dest); screen.ExitLoopClosure()(); }}),
            Button({"Submit - 5", [&] { add5_config(&confi, name, source, dest, source2, dest2); screen.ExitLoopClosure()(); }}),
        }),
    });
    auto filler = Renderer([&] {
      auto fill = hbox({
                    hbox({
                      text(""),
                    }),
                  });
        
      return fill;
    });
    auto tab_container = Container::Tab({
        vmenu_nonsudo,
        vmenu_sudo,
        input_form,
    }, &tab_selected);
    auto global = Container::Vertical({
      tab_toggle,
      tab_container
    });
    auto renderer = Renderer(global, [&] {
      return vbox({
        tab_toggle->Render(),
        separator(),
        tab_container->Render(),
        guide->Render(),
      }) | border;
    });
    renderer |= CatchEvent([&](Event event) {
      if (event == Event::Character('q') && tab_selected != 2) {
        screen.ExitLoopClosure()();
        return true;
      }
      if (event == Event::Character('u') && tab_selected != 2) {
        remove_config(&confi, nonsudo[selected]);
        return true;
      }
      if ((event == Event::Character(' ') || event == Event::Character('\n') || event == Event::Character('i')) && tab_selected != 2) {
        install_config(&confi, nonsudo[selected], false);
        return true;
      }
      if (event == Event::Character('f') && tab_selected != 2) {
        install_config(&confi, nonsudo[selected], true);
        return true;
      }
      return false;
    });
    screen.Loop(renderer);
  }
}

Component VMenu(std::vector<std::string>* entries, int* selected, vector<config>* c) {
  auto option = MenuOption::Vertical();
  option.entries_option.transform = [c](EntryState state) {
    Element e = state.active ? text("[" + state.label + "]")
                             : text(" " + state.label + " ");
    config_list listt = list_config_tui(c);
    if (find(listt.installed.begin(), listt.installed.end(), state.label) != listt.installed.end()){
      e = e | color(Color::Green); 
    }
    
    if (state.focused)
      e = e | bold;

    if (state.focused)
      e = e | bold;
    if (state.active)
      e = e | bold;
    return e;
  };
  return Menu(entries, selected, option);
}
