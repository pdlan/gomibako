#include <string>
#include <iostream>
#include <ctime>
#ifdef _WIN32
#else
#include <unistd.h>
#endif
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
    this->config_filename = config_filename;
    string theme_path;
    std::vector<std::map<string, string>> _users;
    if (!extract_yaml_map(
        config,
        make_pair("ip", &this->ip),
        make_pair("port", &this->port),
        make_pair("theme", &theme_path),
        make_pair("site-name", &this->site_information.name),
        make_pair("site-url", &this->site_information.url),
        make_pair("site-description", &this->site_information.description),
        make_pair("users", &_users)
    )) {
        cerr << "Bad configuration file.\n";
        return false;
    }
    for (auto &&i : _users) {
        auto it1 = i.find("username");
        auto it2 = i.find("password");
        if (it1 == i.end() || it2 == i.end()) {
            cerr << "Bad configuration file.\n";
            return false;
        }
        this->users[it1->second] = it2->second;
    }
    this->url_maker.reset(new SimpleURLMaker(this->site_information.url));
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
    this->site_information.pages = &this->page_manager->get_pages();
    this->site_information.tags = &this->article_manager->get_tags();
    this->article_pager.reset(new Pager(theme_config.articles_per_page));
    this->tag_pager.reset(new Pager(theme_config.articles_per_page_tag));
    this->archives_pager.reset(new Pager(theme_config.articles_per_page_archives));
    this->tag_filter.reset(new CachedFilter<std::string>([] (const std::string &arg, const ArticleMetadata &metadata) {
        return metadata.tags.count(arg);
    }));
    this->theme_static_handler.reset(new StaticHandler(theme_config.static_directory, theme_config.static_files));
    this->admin_static_handler.reset(new StaticHandler("assets/admin/static", {
        {"css/bootstrap.min.css", "text/css"},
        {"css/style.css", "text/css"},
        {"js/bootstrap.min.js", "text/javascript"},
        {"js/jquery.min.js", "text/javascript"},
        {"js/article.js", "text/javascript"},
        {"js/draft.js", "text/javascript"},
        {"js/page.js", "text/javascript"},
        {"js/config.js", "text/javascript"},
        {"fonts/glyphicons-halflings-regular.eot", "application/vnd.ms-fontobject"},
        {"fonts/glyphicons-halflings-regular.ttf", "application/font-sfnt"},
        {"fonts/glyphicons-halflings-regular.woff2", "font/woff2"},
        {"fonts/glyphicons-halflings-regular.svg", "image/svg+xml"},
        {"fonts/glyphicons-halflings-regular.woff", "application/font-woff"}
    }));
    CROW_ROUTE(app, "/static/<path>")([this] (const std::string &filename) {
        return this->theme_static_handler->handle(filename);
    });
    CROW_ROUTE(app, "/admin/static/<path>")([this] (const std::string &filename) {
        return this->admin_static_handler->handle(filename);
    });
    CROW_ROUTE(app, "/article/<string>")(handler_article);
    CROW_ROUTE(app, "/page/c<string>")(handler_custom_page);
    CROW_ROUTE(app, "/page/<uint>")(handler_page);
    CROW_ROUTE(app, "/")(std::bind(handler_page, 1));
    CROW_ROUTE(app, "/tag/<string>/page/<uint>")(handler_tag);
    CROW_ROUTE(app, "/tag/<string>/")(std::bind(handler_tag, placeholders::_1, 1));
    CROW_ROUTE(app, "/archives/page/<uint>")(handler_archives);
    CROW_ROUTE(app, "/archives/")(std::bind(handler_archives, 1));
    CROW_ROUTE(app, "/tags")(handler_tags);
    CROW_ROUTE(app, "/admin/")(handler_admin);
    CROW_ROUTE(app, "/admin/article/page/<uint>")(handler_admin_article);
    CROW_ROUTE(app, "/admin/article/")(std::bind(handler_admin_article, 1));
    CROW_ROUTE(app, "/admin/article/new").methods("POST"_method)(handler_admin_article_new);
    CROW_ROUTE(app, "/admin/article/edit").methods("POST"_method)(handler_admin_article_edit);
    CROW_ROUTE(app, "/admin/article/delete/<string>")(handler_admin_article_delete);
    CROW_ROUTE(app, "/admin/article/json/<string>")(handler_admin_article_json);
    CROW_ROUTE(app, "/admin/article/move/<string>")(handler_admin_article_move);
    CROW_ROUTE(app, "/admin/draft/page/<uint>")(handler_admin_draft);
    CROW_ROUTE(app, "/admin/draft/")(std::bind(handler_admin_draft, 1));
    CROW_ROUTE(app, "/admin/draft/new").methods("POST"_method)(handler_admin_draft_new);
    CROW_ROUTE(app, "/admin/draft/edit").methods("POST"_method)(handler_admin_draft_edit);
    CROW_ROUTE(app, "/admin/draft/delete/<string>")(handler_admin_draft_delete);
    CROW_ROUTE(app, "/admin/draft/json/<string>")(handler_admin_draft_json);
    CROW_ROUTE(app, "/admin/draft/publish").methods("POST"_method)(handler_admin_draft_publish);
    CROW_ROUTE(app, "/admin/page/")(handler_admin_page);
    CROW_ROUTE(app, "/admin/page/new").methods("POST"_method)(handler_admin_page_new);
    CROW_ROUTE(app, "/admin/page/edit").methods("POST"_method)(handler_admin_page_edit);
    CROW_ROUTE(app, "/admin/page/delete/<string>")(handler_admin_page_delete);
    CROW_ROUTE(app, "/admin/page/json/<string>")(handler_admin_page_json);
    CROW_ROUTE(app, "/admin/config/")(handler_admin_config);
    CROW_ROUTE(app, "/admin/config/edit").methods("POST"_method)(handler_admin_config_edit);
    crow::mustache::set_base("assets/admin/template/");
    return true;
}

bool Gomibako::start() {
    this->start_time = time(nullptr);
    this->app.port(this->port).bindaddr(this->ip).run();
    return true;
}

time_t Gomibako::get_uptime() const {
    return time(nullptr) - this->start_time;
}