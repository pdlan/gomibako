#ifndef GOMIBAKO_INCLUDE_THEME_H
#define GOMIBAKO_INCLUDE_THEME_H
#include <string>
#include <sstream>
#include <map>
#include <memory>
#include <functional>
#include "article.h"
#include "url.h"

namespace gomibako {
struct ThemeConfiguration {
    int articles_per_page;
    int articles_per_page_tag;
    int articles_per_page_archives;
    std::string static_directory;
    std::map<std::string, std::string> static_files;
};

struct SiteInformation;

class Theme {
public:
    Theme(const std::string &path);
    ~Theme();
    std::function<void (std::ostringstream &, const ArticleMetadata &, const std::string &,
                        const SiteInformation &, std::shared_ptr<URLMaker> )> render_article;
    bool load();
    const ThemeConfiguration & get_configuration();
private:
    std::string path;
    ThemeConfiguration config;
    void *handle;
};
}
#endif