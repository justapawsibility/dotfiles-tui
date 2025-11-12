#include "cli.hpp"


bool compareConfig(const config& a, const config& b) {
  return a.name < b.name;
}

void sort_configs(vector<config>& configs) {
  sort(configs.begin(), configs.end(), compareConfig);
}

config_lista load_config() {
    vector<config> configurations;
    config_lista config_list;
    string buffer;
    ifstream FILE(CONF_FILE);
    while (getline(FILE, buffer)) {
      vector<string> parts = split(buffer, ",");
      if (parts.size() < 5) {
          cerr << "Invalid config line: " << buffer << endl;
          continue; // skip malformed lines
      }
      config temp {
          .name    = parts[0],
          .source  = parts[1],
          .dest    = parts[2],
          .source2 = parts[3],
          .dest2   = parts[4]
      };
      configurations.push_back(std::move(temp));
    }
    FILE.close();
    sort_configs(configurations);
    config_list.configurations = configurations;
    for (const auto& confi : configurations) {
      if (confi.name.find('*') != string::npos) {
        config_list.sudo.push_back(confi.name);
      }
      else {
        config_list.nonsudo.push_back(confi.name);
      }
    }
    return config_list;
}

void write_config(vector<config>* c) {
  ofstream FILE(CONF_FILE);
  sort_configs(*c);
  for (const auto& confi : *c) {
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
  for (const auto& confi : *c) {
    if (confi.name != name) {
      temp.push_back(confi);
    }
    else {
      path temp_src = weakly_canonical(replace_home(confi.source));
      path temp_src2 = weakly_canonical(replace_home(confi.source2));
      remove_all(temp_src);
      remove_all(temp_src2);
    }
  }
  write_config(&temp);
}

void print_config(vector<config>* configs, const string name) {
  for (const auto& confi : *configs) {
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
  for (const auto& confi : *configs) {
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
  for (const auto& confi : *configs) {
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

void remove_symlink(const path& p) {
    if (is_symlink(p)) {
        remove(p);
        cout << p << " was removed (symlink)" << endl;
    } else if (!p.empty()) {
        cerr << p << " is not a symlink!" << endl;
    }
}

void remove_config(vector<config>* configs, const string& name) {
    for (const auto& confi : *configs) {
        if (name == confi.name) {
            remove_symlink(replace_home(confi.dest));
            remove_symlink(replace_home(confi.dest2));
        }
    }
}
