#ifndef CLI
#define CLI

#include <filesystem>
#include <vector>
#include <regex>
#include <unistd.h>
#include <glok/main.hpp>

#define CONF_FILE "configurations.toml"

using namespace std;
using namespace filesystem;

void remove_symlink(const path& p) {
    if (is_symlink(p)) {
        remove(p);
        cout << p << " was removed (symlink)" << endl;
    } else if (!p.empty()) {
        cerr << p << " is not a symlink!" << endl;
    }
}

struct Config {
  string name;
  path source;
  path dest;

    Config(string n, path s, path d) : name(n), source(s), dest(d) {}

    virtual void install(bool forced) {
      path temp_source = weakly_canonical(replace_home(source));
      path temp_dest = replace_home(dest);

      if (dest != "") {
        if (forced) {
          cout << "force flag enabled, removing " << dest << endl;
        remove_all(temp_dest);
        }
        if (!temp_dest.parent_path().empty()) {
          create_directories(temp_dest.parent_path());
        }
        try {
          create_symlink(temp_source, temp_dest);
          cout << "Creating symlink " << temp_source.string() << " -> " << temp_dest.string() << endl;
        }
        catch (const exception& err) {
          cerr << err.what() << endl;
        }
      }

    }

    virtual void remove() {
      remove_symlink(replace_home(dest));
    }

    virtual void print() {
      cout << "NAME: " << name << endl;
      cout << "SOURCE: " << source << endl;
      cout << "DEST: " << dest << endl;
    }
};

struct Config5 : Config {
  path source2;
  path dest2;
  
  Config5(string n, path s, path d, path s2, path d2) : Config(n, s, d), source2(s2), dest2(d2) {}

  void install(bool forced) override {
    Config::install(forced);
    path temp_source2 = weakly_canonical(replace_home(source2));
    path temp_dest2 = replace_home(dest2);

    if (temp_dest2 != "") {
      if (forced) {
        cout << "force flag enabled, removing " << temp_dest2 << endl;
      remove_all(temp_dest2);
      }
      if (!temp_dest2.parent_path().empty()) {
        create_directories(temp_dest2.parent_path());
      }
      try {
        create_symlink(temp_source2, temp_dest2);
        cout << "Creating symlink " << temp_source2.string() << " -> " << temp_dest2.string() << endl;
      }
      catch (const exception& err) {
        cerr << err.what() << endl;
      }
    }
  }

    void remove() override {
      Config::remove();
      remove_symlink(replace_home(dest2));
    }

    void print() override {
      Config::print();
      cout << "SOURCE2: " << source2 << endl;
      cout << "DEST2: " << dest2 << endl;
    }
};

#endif 
