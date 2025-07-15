#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <regex>
#include <unistd.h>
#include "include/useful.hpp"
#include <argparse/argparse.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#define CONF_FILE "configurations.csv"

using namespace std;
using namespace filesystem;
using namespace ftxui;

struct config {
    public:
        string name;
        path source;
        path dest;
        path source2;
        path dest2;
};

vector<config> load_config() {
    vector<config> configurations;
    string buffer;
    ifstream FILE(CONF_FILE);
    while(getline (FILE, buffer)) {
        config temp;
        vector<string> aftersplit = split(buffer, ",");
        temp.name = aftersplit[0];
        temp.source = aftersplit[1];
        temp.dest = aftersplit[2];
        temp.source2 = aftersplit[3];
        temp.dest2 = aftersplit[4];
        configurations.push_back(temp);
    }
    FILE.close();
    return configurations;
}

void write_config(vector<config>* c) {
  ofstream FILE(CONF_FILE);
  for (config confi : *c) {
    FILE << confi.name << "," << confi.source.string() << "," << confi.dest.string() << "," << confi.source2.string() << "," << confi.dest2.string() << endl;
  }
  FILE.close();
}

void add3_config(vector<config>* c, string name, string source, string dest) {
  config temp;
  temp.name = name;
  temp.source = source;
  temp.dest = revert_home(dest);
  c->push_back(temp);
  write_config(c);
}

void add5_config(vector<config>* c, string name, string source, string dest, string source2, string dest2) {
  config temp;
  temp.name = name;
  temp.source = source;
  temp.dest = revert_home(dest);
  temp.source2 = source2;
  temp.dest2 = revert_home(dest2);
  c->push_back(temp);
  write_config(c);
}

void delete_config(vector<config>* c, const string name) {
  vector<config> temp;
  for (config confi : *c) {
    if (confi.name != name) {
      temp.push_back(confi);
    }
  }
  write_config(&temp);
}

void print_config(vector<config>* configs, const string name) {
  for (config confi : *configs) {
    if (name == confi.name) {
      cout << "NAME: " << confi.name << endl;
      cout << "SOURCE: " << confi.source << endl;
      cout << "DEST: " << confi.dest << endl;
      cout << "SOURCE2: " << confi.source2 << endl;
      cout << "DEST2: " << confi.dest2 << endl;
    }
  }
}

void list_config(vector<config>* configs) {
  vector<string> installed;
  vector<string> not_installed;
  for (config confi : *configs) {
      path temp_dest = replace_home(confi.dest);
      path temp_dest2 = replace_home(confi.dest2);
      if (is_symlink(temp_dest)) {
        installed.push_back(confi.name);
      }
      else {
        not_installed.push_back(confi.name);
      }
  }
  cout << "--------INSTALLED-------" << endl;
  for (string i : installed) {
    cout << i << endl;
  }
  cout << "--------NOT INSTALLED--------" << endl;
  for (string n : not_installed) {
    cout << n << endl;
  }
  
}

void install_config(vector<config>* configs, const string name, bool forced) {
  for (config confi : *configs) {
    if (name == confi.name) {
      path temp_src = weakly_canonical(replace_home(confi.source));
      path temp_dest = replace_home(confi.dest);
      path temp_src2 = weakly_canonical(replace_home(confi.source2));
      path temp_dest2 = replace_home(confi.dest2);
      if (temp_dest != "") {
        if (forced) {
          cout << "force flag enabled, removing " << temp_dest << endl;
          remove_all(temp_dest);
        }
        create_directories(temp_dest.parent_path());
        try {
          create_symlink(temp_src, temp_dest);
          cout << "Creating symlink " << temp_src.string() << " -> " << temp_dest.string() << endl;
        }
        catch (const exception& err) {
          cerr << err.what() << endl;
        }
      }
      if (temp_dest2 != "") {
        if (forced) {
          cout << "force flag enabled, removing " << temp_dest2 << endl;
          remove_all(temp_dest2);
        }
        create_directories(temp_dest2.parent_path());
        try {
          create_symlink(temp_src2, temp_dest2);
          cout << "Creating symlink " << temp_src2.string() << " -> " << temp_dest2.string() << endl;
        }
        catch (const exception& err) {
          cerr << err.what() << endl;
        }
      }
    }
  }
}

void remove_config(vector<config>* configs, const string name) {
  for (config confi : *configs) {
    if (name == confi.name) {
      path temp_dest = replace_home(confi.dest);
      path temp_dest2 = replace_home(confi.dest2);
      if (is_symlink(temp_dest)) {
        remove(temp_dest);
        cout << temp_dest << " was removed (symlink)" << endl;
      }
      else if (is_directory(temp_dest)) {
        cerr << temp_dest << " is a directory!" << endl;
        exit(1);
      }
      else if (is_regular_file(temp_dest)) {
        cerr << temp_dest << " is a regular file!" << endl;
        exit(1);
      }
      if (is_symlink(temp_dest2)) {
        remove(temp_dest2);
        cout << temp_dest2 << " was removed (symlink)" << endl;
      }
      else if (temp_dest2 != "") {
        cerr << temp_dest2 << " is not a symlink!" << endl;
        exit(1);
      }
      else if (is_directory(temp_dest2)) {
        cerr << temp_dest2 << " is a directory!" << endl;
        exit(1);
      }
      else if (is_regular_file(temp_dest2)) {
        cerr << temp_dest2 << " is a regular file!" << endl;
        exit(1);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  vector<config> confi = load_config();
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

  if (program["-q"] == true) {
    if (argc == 2) {
      cerr << program;
      exit(1);
    }
    if (program["-f"] == true) {
      forced = true;
    }
    if (program["-l"] == true) {
      list_config(&confi);
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
          confi = load_config();
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
            confi = load_config();
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
    Element document = hbox({
      text("left")   | border,
      text("middle") | border | flex,
      text("right")  | border,
    });
   
    auto screen = Screen::Create(
      Dimension::Full(),       
      Dimension::Fit(document) 
    );
   
    Render(screen, document);
   
    screen.Print();
  }
}
