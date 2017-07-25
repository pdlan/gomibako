#ifndef GOMIBAKO_INCLUDE_THEME_H
#define GOMIBAKO_INCLUDE_THEME_H
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <memory>
#include <functional>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include "article.h"
#include "url.h"

namespace gomibako {
struct ThemeConfiguration {
    int articles_per_page;
    int articles_per_page_tag;
    int articles_per_page_archives;
    std::string static_directory;
    std::map<std::string, std::string> static_files;
    std::set<int> error_codes;
};

struct SiteInformation;

class Theme {
public:
    Theme(const std::string &path);
    ~Theme();
    std::function<void (std::ostringstream &, const ArticleMetadata &, const std::string &,
                        const SiteInformation &, std::shared_ptr<URLMaker> )> render_article;
    std::function<void (std::ostringstream &, const std::vector<ArticleMetadata> &,
                        const std::vector<std::ostringstream> &, int, int,
                        const SiteInformation &, std::shared_ptr<URLMaker>)> render_page;
    std::function<void (std::ostringstream &, const std::vector<ArticleMetadata> &, int, int,
                        const SiteInformation &, std::shared_ptr<URLMaker>)> render_archives;
    std::function<void (std::ostringstream &, const std::vector<ArticleMetadata> &,
                        const std::vector<std::ostringstream> &, const std::string &, int, int,
                        const SiteInformation &, std::shared_ptr<URLMaker>)> render_tag;
    std::function<void (std::ostringstream &, const CustomPage &, const SiteInformation &,
                        std::shared_ptr<URLMaker>)> render_custom_page;
    std::function<void (std::ostringstream &, const std::map<std::string, int> &,
                        const SiteInformation &, std::shared_ptr<URLMaker>)> render_tags;
    std::function<void (std::ostringstream &, int code, const SiteInformation &,
                        std::shared_ptr<URLMaker>)> render_error;
    bool load();
    const ThemeConfiguration & get_configuration();
private:
    std::string path;
    ThemeConfiguration config;
#ifdef _WIN32
	HMODULE handle;
#else
    void *handle;
#endif
};
}
#endif