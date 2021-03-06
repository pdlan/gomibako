#ifndef GOMIBAKO_INCLUDE_GOMIBAKO_H
#define GOMIBAKO_INCLUDE_GOMIBAKO_H
#include <string>
#include <memory>
#include <crow.h>
#include "pager.h"
#include "article.h"
#include "theme.h"
#include "filter.h"
#include "url.h"
#include "handlers.h"

namespace gomibako {
struct SiteInformation {
    std::string name;
    std::string url;
    std::string description;
    std::string author;
    const std::vector<CustomPage> *pages;
    const std::map<std::string, int> *tags;
    std::vector<ArticleMetadata> recent_articles;
    std::function<void (int)> get_recent_articles;
};

class Gomibako {
public:
    static Gomibako & get_instance();
    bool initialize(const std::string &config_filename);
    bool start();
    std::shared_ptr<ArticleManager> get_article_manager() {return this->article_manager;}
    std::shared_ptr<ArticleManager> get_draft_manager() {return this->draft_manager;}
    std::shared_ptr<PageManager> get_page_manager() {return this->page_manager;}
    std::shared_ptr<Pager> get_article_pager() const {return this->article_pager;}
    std::shared_ptr<Pager> get_tag_pager() const {return this->tag_pager;}
    std::shared_ptr<Pager> get_archives_pager() const {return this->archives_pager;}
    std::shared_ptr<CachedFilter<std::string>> get_tag_filter() {return this->tag_filter;}
    std::shared_ptr<Theme> get_theme() const {return this->theme;}
    std::shared_ptr<URLMaker> get_url_maker() const {return this->url_maker;}
    const SiteInformation & get_site_information() const {return this->site_information;}
    const std::map<std::string, std::string> & get_users() const {return this->users;}
    time_t get_uptime() const;
    std::string get_config_filename() const {return this->config_filename;}
private:
    std::shared_ptr<Theme> theme;
    std::shared_ptr<URLMaker> url_maker;
    std::shared_ptr<ArticleManager> article_manager;
    std::shared_ptr<ArticleManager> draft_manager;
    std::shared_ptr<PageManager> page_manager;
    std::shared_ptr<Pager> article_pager;
    std::shared_ptr<Pager> tag_pager;
    std::shared_ptr<Pager> archives_pager;
    std::shared_ptr<CachedFilter<std::string>> tag_filter;
    std::shared_ptr<StaticHandler> theme_static_handler;
    std::shared_ptr<StaticHandler> admin_static_handler;
    SiteInformation site_information;
    std::string config_filename;
    std::string ip;
    uint16_t port;
    std::map<std::string, std::string> users;
    crow::App<BasicAuth, ErrorHandler> app;
    time_t start_time;
};
}
#endif