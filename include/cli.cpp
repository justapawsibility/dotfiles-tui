#include "cli.hpp"


bool compareConfig(const config& a, const config& b) {
  return a.name < b.name;
}

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
    sort(configurations.begin(), configurations.end(), compareConfig);
    return configurations;
}

void write_config(vector<config>* c) {
  ofstream FILE(CONF_FILE);
  sort(c->begin(), c->end(), compareConfig);
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
