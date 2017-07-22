#include <string>
#include <iostream>
#include "gomibako.h"
#include "handlers.h"
#include "util/yaml.h"

using namespace gomibako;

Gomibako & Gomibako::get_instance() {
    static Gomibako gomibako;
    return gomibako;
}

bool Gomibako::initialize(const std::string &config_filename) {
    using namespace std;
    YAML::Node config;
    try {
        config = YAML::LoadFile(config_filename);
    } catch (const YAML::BadFile &e) {
        cerr << "No such file.\n";
        return false;
    }
    string theme_path;
    if (!extract_yaml_map(
        config,
        make_pair("ip", &this->ip),
        make_pair("port", &this->port),
        make_pair("theme", &theme_path),
        make_pair("site-name", &this->site_information.name),
        make_pair("site-url", &this->site_information.url),
        make_pair("site-description", &this->site_information.description)
    )) {
        cerr << "Bad configuration file.\n";
        return false;
    }
    this->url_maker.reset(new URLMaker(this->site_information.url));
    this->theme.reset(new Theme("theme/" + theme_path));
    if (!this->theme->load()) {
        cerr << "Cannot load theme.\n";
        return false;
    }
    const ThemeConfiguration &theme_config = this->theme->get_configuration();
    this->article_manager.reset(new ArticleManager("articles"));
    this->draft_manager.reset(new ArticleManager("drafts"));
    this->page_manager.reset(new PageManager("pages.yaml"));
    if (!this->article_manager->load_metadata() ||
        !this->draft_manager->load_metadata() ||
        !this->page_manager->load_pages()) {
        return false;
    }
    this->site_information.pages = this->page_manager->get_pages();
    this->article_pager.reset(new Pager(theme_config.articles_per_page));
    this->tag_pager.reset(new Pager(theme_config.articles_per_page_tag));
    this->archives_pager.reset(new Pager(theme_config.articles_per_page_archives));
    this->tag_filter.reset(new CachedFilter<std::string>([] (const std::string &arg, const ArticleMetadata &metadata) {
        return metadata.tags.count(arg);
    }));
    this->theme_static_handler.reset(new StaticHandler(theme_config.static_directory, theme_config.static_files));
    CROW_ROUTE(app, "/static/<string>")([this] (const std::string &filename) {
        return this->theme_static_handler->handle(filename);
    });
    CROW_ROUTE(app, "/article/<string>/")(handler_article);
    CROW_ROUTE(app, "/page/<int>/")(handler_page);
    CROW_ROUTE(app, "/")(std::bind(handler_page, 1));
    return true;
}

bool Gomibako::start() {
    this->app.port(this->port).run();
    return true;
}