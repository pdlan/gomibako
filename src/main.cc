#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <ctime>
#include <cstring>
#include "gomibako.h"
#include "pager.h"
#include "util/yaml.h"
#include "util/picosha2.h"

using namespace gomibako;

bool has_file(const char *filename) {
    std::ifstream fs(filename);
    if (fs) {
        fs.close();
        return true;
    } else {
        return false;
    }
}

bool write_to_file(const char *filename, const char *content) {
    std::ofstream fs(filename);
    if (!fs) {
        return false;
    }
    fs << content;
    fs.close();
    return true;
}

void print_help() {
    std::cout <<
R"(Gomibako is a simple C++ blog program.
Usage:
./gomibako run [config=config.yaml]
./gomibako install
./gomibako help
)";
}

void install() {
    using namespace std;
    string site_name, site_url, site_description, ip, username, password, theme;
    int port;
    cout << "Please answer the following questions to generate a configuration file.\n";
    cout << "Site name:";
    getline(cin, site_name);
    cout << "Site url (No '/', e.g.: http://localhost):";
    getline(cin, site_url);
    cout << "Site description:";
    getline(cin, site_description);
    cout << "Bind ip (If you use reverse proxy, input 127.0.0.1, otherwise input 0.0.0.0):";
    getline(cin, ip);
    cout << "Port:";
    cin >> port;
    cout << "Administrator's username:";
    getline(cin >> ws, username);
    cout << "Administrator's password:";
    getline(cin, password);
    string password_hash;
    picosha2::hash256_hex_string(password, password_hash);
    cout << "Theme:";
    getline(cin, theme);
    YAML::Emitter out;
    out << YAML::BeginMap
        << YAML::Key << "site-name" << YAML::Value << site_name
        << YAML::Key << "site-url" << YAML::Value << site_url
        << YAML::Key << "site-description" << YAML::Value << site_description
        << YAML::Key << "ip" << YAML::Value << ip
        << YAML::Key << "port" << YAML::Value << port
        << YAML::Key << "theme" << YAML::Value << theme
        << YAML::Key << "users" << YAML::Value
        << YAML::BeginSeq << YAML::BeginMap
        << YAML::Key << "username" << YAML::Value << username
        << YAML::Key << "password" << YAML::Value << password_hash
        << YAML::EndMap << YAML::EndSeq
        << YAML::EndMap;
    if (!write_to_file("config.yaml", out.c_str())) {
        cout << "Failed to write to configuration file.\n";
        return;
    }
    if (!has_file("articles/metadata.yaml")) {
        write_to_file("articles/metadata.yaml", "[]");
    }
    if (!has_file("drafts/metadata.yaml")) {
        write_to_file("drafts/metadata.yaml", "[]");
    }
    if (!has_file("pages.yaml")) {
        write_to_file("pages.yaml", "[]");
    }
    cout << "config.yaml has been generated. run \"./gomibako run\" to start gomibako.\n";
}

int main(int argc, const char **argv) {
    if (argc != 2 && argc != 3 && argc != 4) {
        print_help();
        return 0;
    }
    const char *command = argv[1];
    if (strcmp(command, "run") == 0) {
        Gomibako &gomibako = Gomibako::get_instance();
        bool success = false;
        if (argc == 2) {
            success = gomibako.initialize("config.yaml");
        } else if (argc == 3) {
            success = gomibako.initialize(argv[2]);
        }
        if (!success) {
            return 0;
        }
        gomibako.start();
    } else if (strcmp(command, "help") == 0 && argc == 2) {
        print_help();
        return 0;
    } else if (strcmp(command, "install") == 0 && argc == 2) {
        install();
        return 0;
    } else {
        print_help();
        return 0;
    }
    return 0;
}