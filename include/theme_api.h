#ifndef GOMIBAKO_INCLUDE_THEME_API_H
#define GOMIBAKO_INCLUDE_THEME_API_H
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <set>

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
    std::set<int> error_codes;
};

class URLMaker {
public:
    virtual std::string url_article(const std::string &id) = 0;
    virtual std::string url_page(int page) = 0;
    virtual std::string url_tag(const std::string &tag, int page = 1) = 0;
    virtual std::string url_index() = 0;
    virtual std::string url_archives(int page = 1) = 0;
    virtual std::string url_static(const std::string &path) = 0;
    virtual std::string url_custom_page(const std::string &id) = 0;
    virtual std::string url_tags() = 0;
};
}
#endif