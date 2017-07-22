#ifndef GOMIBAKO_INCLUDE_THEME_API_H
#define GOMIBAKO_INCLUDE_THEME_API_H
#include <string>
#include <map>
#include <vector>
#include <set>
#include "url.h"

namespace gomibako {
struct ArticleMetadata {
    std::string id;
    std::string title;
    std::string filename;
    time_t timestamp;
    std::set<std::string> tags;
};

struct CustomPage {
    int order;
    std::string title;
    std::string id;
    std::string content;
};

struct SiteInformation {
    std::string name;
    std::string url;
    std::string description;
    const std::vector<CustomPage> *pages;
};

struct ThemeConfiguration {
    int articles_per_page;
    int articles_per_page_tag;
    int articles_per_page_archives;
    std::string static_directory;
    std::map<std::string, std::string> static_files;
};
}
#endif