#ifndef CLI
#define CLI

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <regex>
#include <unistd.h>
#include <cpp_lib_personal/main.hpp>

#define CONF_FILE "configurations.csv"

using namespace std;
using namespace filesystem;

struct config {
  public:
      string name;
      path source;
      path dest;
      path source2;
      path dest2;
};

struct config_lista {
  public:
    vector<config> configurations;
    vector<string> nonsudo;
    vector<string> sudo;
};

config_lista load_config();
void write_config(vector<config>* c);
void add3_config(vector<config>* c, string name, string source, string dest);
void add5_config(vector<config>* c, string name, string source, string dest, string source2, string dest2);
void delete_config(vector<config>* c, const string name);
void print_config(vector<config>* configs, const string name);
void list_config(vector<config>* configs);
void install_config(vector<config>* configs, const string name, bool forced);
void remove_config(vector<config>* configs, const string& name);

#endif
