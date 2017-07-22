#ifndef GOMIBAKO_INCLUDE_UTIL_YAML_H
#define GOMIBAKO_INCLUDE_UTIL_YAML_H
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <yaml-cpp/yaml.h>

namespace gomibako {
template<typename T>
bool extract_yaml_map(const YAML::Node &node, const std::pair<const char *, T *> &arg) {
    if (!node[arg.first]) {
        return false;
    }
    try {
        *arg.second = node[arg.first].template as<T>();
        return true;
    } catch(const YAML::BadConversion &e) {
        return false;
    }
}

template<typename First, typename... Rest>
bool extract_yaml_map(const YAML::Node &node,
                      const std::pair<const char *, First *> &arg_first,
                      const std::pair<const char *, Rest *> &...args_rest) {
    if (!node[arg_first.first]) {
        return false;
    }
    try {
        *arg_first.second = node[arg_first.first].template as<First>();
    } catch(const YAML::BadConversion &e) {
        return false;
    }
    return extract_yaml_map(node, args_rest...);
}

}
#endif