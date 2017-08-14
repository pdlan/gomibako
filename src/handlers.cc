#include <string>
#include <vector>
#include <sstream>
#include <list>
#include <crow.h>
#include "util/base64.h"
#include "util/picosha2.h"
#include "gomibako.h"
#include "handlers.h"
#include "util/yaml.h"
#include "url.h"

using namespace std;
using namespace gomibako;

crow::response handler_article(const string &id_encoded) {
    const string &id = urldecode(id_encoded);
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    ArticleMetadata metadata;
    if (!article_manager->get_metadata(id, metadata)) {
        return crow::response(404);
    }
    ostringstream content, out;
    article_manager->get_content(id, content);
    gomibako.get_theme()->render_article(out, metadata, content.str(),
                                         gomibako.get_site_information(), gomibako.get_url_maker());
    return crow::response(out.str());
}

crow::response handler_custom_page(const string &id_encoded) {
    const string &id = urldecode(id_encoded);
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<PageManager> page_manager = gomibako.get_page_manager();
    CustomPage page;
    if (!page_manager->get_page(id, page)) {
        return crow::response(404);
    }
    ostringstream out;
    gomibako.get_theme()->render_custom_page(out, page, gomibako.get_site_information(),
                                             gomibako.get_url_maker());
    return crow::response(out.str());
}

crow::response handler_page(int page) {
    if (page == 0) {
        return crow::response(404);
    }
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    shared_ptr<Pager> pager = gomibako.get_article_pager();
    vector<string> ids;
    int pages;
    article_manager->apply_filter(pager->get_filter(page, pages), ids);
    if (page > pages) {
        return crow::response(404);
    }
    vector<ArticleMetadata> metadata;
    article_manager->get_metadata(ids, metadata);
    ostringstream out;
    gomibako.get_theme()->render_page(
        out, metadata,
        [article_manager] (const string &id, ostringstream &content) {
            return article_manager->get_content(id, content);
        },
        page, pages, gomibako.get_site_information(), gomibako.get_url_maker()
    );
    return crow::response(out.str());
}

crow::response handler_tag(const string &tag_encoded, int page) {
    if (page == 0) {
        return crow::response(404);
    }
    const string &tag = urldecode(tag_encoded);
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    shared_ptr<Pager> pager = gomibako.get_tag_pager();
    vector<string> ids;
    int pages;
    article_manager->apply_filters(
        {
            gomibako.get_tag_filter()->get_filter(tag),
            pager->get_filter(page, pages)
        }, ids);
    if (page > pages) {
        return crow::response(404);
    }
    vector<ArticleMetadata> metadata;
    vector<ostringstream> content;
    article_manager->get_metadata(ids, metadata);
    article_manager->get_content(ids, content);
    ostringstream out;
    gomibako.get_theme()->render_tag(
        out, metadata,
        [article_manager] (const string &id, ostringstream &content) {
            return article_manager->get_content(id, content);
        },
        tag, page, pages, gomibako.get_site_information(), gomibako.get_url_maker()
    );
    return crow::response(out.str());
}

crow::response handler_archives(int page) {
    if (page == 0) {
        return crow::response(404);
    }
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    shared_ptr<Pager> pager = gomibako.get_archives_pager();
    vector<string> ids;
    int pages;
    article_manager->apply_filter(pager->get_filter(page, pages), ids);
    if (page > pages) {
        return crow::response(404);
    }
    vector<ArticleMetadata> metadata;
    article_manager->get_metadata(ids, metadata);
    ostringstream out;
    gomibako.get_theme()->render_archives(out, metadata, page, pages,
                                          gomibako.get_site_information(), gomibako.get_url_maker());
    return crow::response(out.str());
}

crow::response handler_tags() {
    Gomibako &gomibako = Gomibako::get_instance();
    ostringstream out;
    gomibako.get_theme()->render_tags(out, gomibako.get_article_manager()->get_tags(),
                                      gomibako.get_site_information(), gomibako.get_url_maker());
    return crow::response(out.str());
}

crow::response handler_feed() {
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    vector<string> recent_articles;
    static Pager pager(10);
    int pages;
    article_manager->apply_filter(
        pager.get_filter(1, pages),
        recent_articles
    );
    vector<ArticleMetadata> metadata;
    vector<ostringstream> content;
    article_manager->get_metadata(recent_articles, metadata);
    article_manager->get_content(recent_articles, content);
    if (metadata.empty()) {
        return crow::response(404);
    }
    if (metadata.size() != content.size()) {
        return crow::response(500);
    }
    const SiteInformation &site_information = gomibako.get_site_information();
    shared_ptr<URLMaker> url_maker = gomibako.get_url_maker();
    ostringstream out;
    out <<
R"(<?xml version="1.0" encoding="utf-8"?>
<feed xmlns="http://www.w3.org/2005/Atom">
    <title>)" << site_information.name << R"(</title>
    <subtitle>)" << site_information.description << R"(</subtitle>
    <link href=")" << url_maker->url_feed() << R"(" rel="self" />
    <link href=")" << url_maker->url_index() << R"(" />
    <updated>)" << put_time(gmtime(&metadata[0].timestamp), "%Y-%m-%dT%H:%M:%SZ") << R"(</updated>
    <id>)" << site_information.url << R"(</id>)";
    for (size_t i = 0; i < metadata.size(); ++i) {
        const ArticleMetadata &m = metadata[i];
        const ostringstream &c = content[i];
        ostringstream xml_datetime;
        xml_datetime << put_time(gmtime(&m.timestamp), "%Y-%m-%dT%H:%M:%SZ");
        out << R"(
    <entry>
        <title><![CDATA[)" << m.title << R"(]]></title>
        <author><name>)" << site_information.author
            << "</name><uri>" << site_information.url << R"(</uri></author>
        <link href=")" << url_maker->url_article(m.id) << R"(" />
        <updated>)" << xml_datetime.str() << R"(</update>
        <published>)" << xml_datetime.str() << R"(</published>
        <id>)" << m.id << R"(</id>
        <content type="html">
            <![CDATA[)" << c.str() << R"(]]>
        </content>
    </entry>)";
    }
    out << "\n</feed>";
    crow::response res(out.str());
    res.set_header("Content-Type", "application/atom+xml");
    return res;
}

crow::response handler_admin() {
    crow::json::wvalue ctx;
    ctx["title"] = "Home | Dashboard";
    time_t uptime = Gomibako::get_instance().get_uptime();
    int day, hour, minute, second;
    day = uptime / (3600 * 24);
    uptime -= day * 3600 * 24;
    hour = uptime / 3600;
    uptime -= hour * 3600;
    minute = uptime / 60;
    second = uptime - minute * 60;
    ostringstream os;
    os << day << " days " << hour << " hours " << minute << " minutes " << second << " seconds";
    ctx["uptime"] = os.str();
    return crow::response(crow::mustache::load("index.html").render(ctx));
}

crow::response handler_admin_article_draft(shared_ptr<ArticleManager> manager, 
                                           const char *tpl, int page) {
    if (page == 0) {
        return crow::response(404);
    }
    vector<string> ids;
    int pages;
    static Pager pager(10);
    manager->apply_filter(pager.get_filter(page, pages), ids);
    if (page > pages && pages != 0) {
        return crow::response(404);
    }
    vector<ArticleMetadata> metadata;
    manager->get_metadata(ids, metadata);
    crow::json::wvalue ctx;
    ctx["editor"] = true;
    if (strcmp(tpl, "article.html") == 0) {
        ctx["article_js"] = true;
        ctx["title"] = "Article | Dashboard";
    } else {
        ctx["draft_js"] = true;
        ctx["title"] = "Draft | Dashboard";
    }
    for (size_t i = 0; i < metadata.size(); ++i) {
        crow::json::wvalue article;
        ctx["article"][i]["id"] = metadata[i].id;
        ctx["article"][i]["id_encoded"] = urlencode(metadata[i].id.c_str(), metadata[i].id.length());
        ctx["article"][i]["title"] = metadata[i].title;
        ostringstream os;
        os << put_time(localtime(&metadata[i].timestamp), "%Y-%m-%d");
        ctx["article"][i]["time"] = os.str();
    }
    ctx["page"] = page;
    ctx["pages"] = pages;
    ctx["no_prev"] = true;
    ctx["no_next"] = true;
    if (page != 1) {
        ctx["prev_page"] = page - 1;
        ctx["has_prev"] = true;
        ctx["no_prev"] = false;
    }
    if (page != pages && pages != 0) {
        ctx["next_page"] = page + 1;
        ctx["has_next"] = true;
        ctx["no_next"] = false;
    }
    return crow::response(crow::mustache::load(tpl).render(ctx));
}

crow::response handler_admin_article_draft_new(shared_ptr<ArticleManager> manager,
                                               const crow::request &req) {
    YAML::Node node;
    try {
        node = YAML::Load(req.body);
    } catch (const YAML::Exception &e) {
        return crow::response(400);
    }
    string title, content;
    vector<string> tags_vector;
    if (!extract_yaml_map(node,
        make_pair("title", &title),
        make_pair("content", &content),
        make_pair("tags", &tags_vector)
    )) {
        return crow::response(400);
    }
    if (title.empty()) {
        return crow::response(400);
    }
    set<string> tags;
    for (auto &&tag : tags_vector) {
        if (!tags.count(tag)) {
            tags.insert(tag);
        }
    }
    const string &id = manager->add_article(
        title,
        content,
        time(nullptr),
        tags
    );
    return crow::response(id);
}

crow::response handler_admin_article_draft_edit(shared_ptr<ArticleManager> manager,
                                                const crow::request &req) {
    YAML::Node node;
    try {
        node = YAML::Load(req.body);
    } catch (const YAML::Exception &e) {
        return crow::response(400);
    }
    string id, title, content;
    vector<string> tags_vector;
    if (!extract_yaml_map(node,
        make_pair("id", &id),
        make_pair("title", &title),
        make_pair("content", &content),
        make_pair("tags", &tags_vector)
    )) {
        return crow::response(400);
    }
    if (title.empty()) {
        return crow::response(400);
    }
    ArticleMetadata metadata;
    if (!manager->get_metadata(id, metadata)) {
        return crow::response(400);
    }
    set<string> tags;
    for (auto &&tag : tags_vector) {
        if (!tags.count(tag)) {
            tags.insert(tag);
        }
    }
    if (manager->edit_article(
        id,
        title,
        metadata.timestamp,
        tags,
        content
    )) {
        return crow::response("ok");
    } else {
        return crow::response(400);
    }
}

crow::response handler_admin_article_draft_delete(shared_ptr<ArticleManager> manager,
                                                  const string &id_encoded) {
    if (manager->delete_article(urldecode(id_encoded), true)) {
        return crow::response("ok");
    } else {
        return crow::response(400);
    }
}

crow::response handler_admin_article_draft_json(shared_ptr<ArticleManager> manager,
                                                const string &id_encoded) {
    ArticleMetadata metadata;
    const string &id = urldecode(id_encoded);
    if (!manager->get_metadata(id, metadata)) {
        return crow::response(404);
    }
    ostringstream content;
    crow::json::wvalue json;
    manager->get_content(id, content);
    json["title"] = metadata.title;
    json["time"] = metadata.timestamp;
    size_t i = 0;
    json["tags"] = crow::json::rvalue(crow::json::type::List);
    for (auto &&it = metadata.tags.begin(); it != metadata.tags.end(); ++it, ++i) {
        json["tags"][i] = *it;
    }
    json["content"] = content.str();
    return json;
}

crow::response handler_admin_article_move(const string &id_encoded) {
    const string &id = urldecode(id_encoded);
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    shared_ptr<ArticleManager> draft_manager = gomibako.get_draft_manager();
    ArticleMetadata metadata;
    if (!article_manager->get_metadata(id, metadata)) {
        return crow::response(404);
    }
    ostringstream content, out;
    article_manager->get_content(id, content);
    const string &new_id = draft_manager->add_article(metadata.title, content.str(),
                                                      metadata.timestamp, metadata.tags);
    article_manager->delete_article(id, true);
    static vector<string> empty;
    int page, pages;
    draft_manager->apply_filter(get_pagination(new_id, 10, page, pages), empty);
    out << "/admin/draft/page/" << page;
    return crow::response(out.str());
}

crow::response handler_admin_article(int page) {
    Gomibako &gomibako = Gomibako::get_instance();
    return handler_admin_article_draft(gomibako.get_article_manager(), "article.html", page);
}

crow::response handler_admin_article_new(const crow::request &req) {
    crow::response res =
        handler_admin_article_draft_new(Gomibako::get_instance().get_article_manager(), req);
    Gomibako::get_instance().get_tag_filter()->clear_cache();
    return res;
}

crow::response handler_admin_article_edit(const crow::request &req) {
    crow::response res =
        handler_admin_article_draft_edit(Gomibako::get_instance().get_article_manager(), req);
    Gomibako::get_instance().get_tag_filter()->clear_cache();
    return res;
}

crow::response handler_admin_article_delete(const string &id_encoded) {
    crow::response res =
        handler_admin_article_draft_delete(Gomibako::get_instance().get_article_manager(), id_encoded);
    Gomibako::get_instance().get_tag_filter()->clear_cache();
    return res;
}

crow::response handler_admin_article_json(const string &id_encoded) {
    return handler_admin_article_draft_json(Gomibako::get_instance().get_article_manager(), id_encoded);
}

crow::response handler_admin_draft(int page) {
    Gomibako &gomibako = Gomibako::get_instance();
    return handler_admin_article_draft(gomibako.get_draft_manager(), "draft.html", page);
}

crow::response handler_admin_draft_new(const crow::request &req) {
    return handler_admin_article_draft_new(Gomibako::get_instance().get_draft_manager(), req);
}

crow::response handler_admin_draft_edit(const crow::request &req) {
    return handler_admin_article_draft_edit(Gomibako::get_instance().get_draft_manager(), req);
}

crow::response handler_admin_draft_delete(const string &id) {
    return handler_admin_article_draft_delete(Gomibako::get_instance().get_draft_manager(), id);
}

crow::response handler_admin_draft_json(const string &id) {
    return handler_admin_article_draft_json(Gomibako::get_instance().get_draft_manager(), id);
}

crow::response handler_admin_draft_publish(const crow::request &req) {
    YAML::Node node;
    try {
        node = YAML::Load(req.body);
    } catch (const YAML::Exception &e) {
        return crow::response(400);
    }
    string id;
    bool now;
    vector<string> tags_vector;
    if (!extract_yaml_map(node,
        make_pair("id", &id),
        make_pair("now", &now)
    )) {
        return crow::response(400);
    }
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    shared_ptr<ArticleManager> draft_manager = gomibako.get_draft_manager();
    ArticleMetadata metadata;
    if (!draft_manager->get_metadata(id, metadata)) {
        return crow::response(404);
    }
    ostringstream content, out;
    draft_manager->get_content(id, content);
    string new_id;
    if (now) {
        new_id = article_manager->add_article(metadata.title, content.str(),
                                              time(nullptr), metadata.tags);
    } else {
        new_id = article_manager->add_article(metadata.title, content.str(),
                                              metadata.timestamp, metadata.tags);
    }
    draft_manager->delete_article(id, true);
    static vector<string> empty;
    int page, pages;
    article_manager->apply_filter(get_pagination(new_id, 10, page, pages), empty);
    out << "/admin/article/page/" << page;
    return crow::response(out.str());
}

crow::response handler_admin_page() {
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<PageManager> page_manager = gomibako.get_page_manager();
    crow::json::wvalue ctx;
    ctx["page_js"] = true;
    ctx["editor"] = true;
    ctx["title"] = "Page | Dashboard";
    const vector<CustomPage> pages = page_manager->get_pages();
    for (size_t i = 0; i < pages.size(); ++i) {
        const CustomPage &page = pages[i];
        ctx["page"][i]["id"] = page.id;
        ctx["page"][i]["id_encoded"] = urlencode(page.id.c_str(), page.id.length());
        ctx["page"][i]["title"] = page.title;
        ctx["page"][i]["order"] = page.order;
        ctx["page"][i]["content"] = page.content;
    }
    if (pages.size() == 0) {
        ctx["new_order"] = 1;
    } else {
        ctx["new_order"] = pages[pages.size() - 1].order + 1;
    }
    return crow::response(crow::mustache::load("page.html").render(ctx));
}

crow::response handler_admin_page_new(const crow::request &req) {
    YAML::Node node;
    try {
        node = YAML::Load(req.body);
    } catch (const YAML::Exception &e) {
        return crow::response(400);
    }
    int order;
    string title, content;
    if (!extract_yaml_map(node,
        make_pair("order", &order),
        make_pair("title", &title),
        make_pair("content", &content)
    )) {
        return crow::response(400);
    }
    if (title.empty()) {
        return crow::response(400);
    }
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<PageManager> page_manager = gomibako.get_page_manager();
    const string &id = page_manager->add_page(order, title, content);
    return crow::response(id);
}

crow::response handler_admin_page_edit(const crow::request &req) {
    YAML::Node node;
    try {
        node = YAML::Load(req.body);
    } catch (const YAML::Exception &e) {
        return crow::response(400);
    }
    int order;
    string id, title, content;
    if (!extract_yaml_map(node,
        make_pair("order", &order),
        make_pair("id", &id),
        make_pair("title", &title),
        make_pair("content", &content)
    )) {
        return crow::response(400);
    }
    if (title.empty()) {
        return crow::response(400);
    }
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<PageManager> page_manager = gomibako.get_page_manager();
    if (page_manager->edit_page(id, order, title, content)) {
        return crow::response("ok");
    } else {
        return crow::response(400);
    }
}

crow::response handler_admin_page_delete(const string &id_encoded) {
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<PageManager> page_manager = gomibako.get_page_manager();
    if (page_manager->delete_page(urldecode(id_encoded))) {
        return crow::response("ok");
    } else {
        return crow::response(400);
    }
}

crow::response handler_admin_page_json(const string &id_encoded) {
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<PageManager> page_manager = gomibako.get_page_manager();
    const string &id = urldecode(id_encoded);
    CustomPage page;
    if (!page_manager->get_page(id, page)) {
        return crow::response(400);
    }
    crow::json::wvalue json;
    json["id"] = id;
    json["title"] = page.title;
    json["content"] = page.content;
    json["order"] = page.order;
    return json;
}

crow::response handler_admin_config() {
    Gomibako &gomibako = Gomibako::get_instance();
    YAML::Node config;
    try {
        config = YAML::LoadFile(gomibako.get_config_filename());
    } catch (const YAML::BadFile &e) {
        return crow::response(500);
    }
    string site_name, site_url, site_description, ip, theme;
    vector<map<string, string>> users;
    int port;
    if (!extract_yaml_map(
        config,
        make_pair("site-name", &site_name),
        make_pair("site-url", &site_url),
        make_pair("site-description", &site_description),
        make_pair("ip", &ip),
        make_pair("port", &port),
        make_pair("theme", &theme),
        make_pair("users", &users)
    )) {
        return crow::response(500);
    }
    crow::json::wvalue ctx;
    ctx["config_js"] = true;
    ctx["title"] = "Configuration | Dashboard";
    ctx["site-name"] = site_name;
    ctx["site-url"] = site_url;
    ctx["site-description"] = site_description;
    ctx["ip"] = ip;
    ctx["port"] = port;
    ctx["theme"] = theme;
    ctx["users"] = crow::json::rvalue(crow::json::type::List);
    for (size_t i = 0; i < users.size(); ++i) {
        auto &&it = users[i].find("username");
        if (it == users[i].end()) {
            return crow::response(500);
        }
        ctx["users"][i]["username"] = it->second;
        ctx["users"][i]["username_encoded"] = urlencode(it->second.c_str(), it->second.length());
    }
    return crow::response(crow::mustache::load("config.html").render(ctx));
}

crow::response handler_admin_config_edit(const crow::request &req) {
    Gomibako &gomibako = Gomibako::get_instance();
    YAML::Node node_req, node_orig;
    try {
        node_req = YAML::Load(req.body);
        node_orig = YAML::LoadFile(gomibako.get_config_filename());
    } catch (const YAML::Exception &e) {
        return crow::response("failed");
    }
    vector<map<string, string>> users_orig, users;
    if (!extract_yaml_map(node_orig, make_pair("users", &users_orig))) {
        return crow::response("failed");
    }
    string site_name, site_url, site_description, ip, theme;
    int port;
    YAML::Node users_diff;
    vector<map<string, string>> new_users;
    vector<string> delete_users_vec;
    set<string> delete_users;
    if (!extract_yaml_map(
        node_req,
        make_pair("site-name", &site_name),
        make_pair("site-url", &site_url),
        make_pair("site-description", &site_description),
        make_pair("ip", &ip),
        make_pair("port", &port),
        make_pair("theme", &theme)
    ) || !node_req["users"]) {
        return crow::response("failed");
    }
    users_diff = node_req["users"];
    if (!extract_yaml_map(
        users_diff,
        make_pair("new", &new_users),
        make_pair("delete", &delete_users_vec)
    )) {
        return crow::response("failed");
    }
    for (auto &&i : delete_users_vec) {
        if (!delete_users.count(i)) {
            delete_users.insert(i);
        }
    }
    for (auto &&i : users_orig) {
        auto &&it1 = i.find("username");
        auto &&it2 = i.find("password");
        if (it1 == i.end() || it2 == i.end()) {
            return crow::response("failed");
        }
        if (!delete_users.count(it1->second)) {
            users.push_back({
                {"username", it1->second},
                {"password", it2->second}
            });
        }
    }
    for (auto &&i : new_users) {
        auto &&it1 = i.find("username");
        auto &&it2 = i.find("password");
        if (it1 == i.end() || it2 == i.end()) {
            return crow::response("failed");
        }
        const string &username = it1->second;
        string password_hash;
        picosha2::hash256_hex_string(it2->second, password_hash);
        if (find_if(users.begin(), users.end(), [&username] (const map<string, string> &p) {
            return p.at("username") == username;
        }) == users.end()) {
            users.push_back({
                {"username", username},
                {"password", password_hash}
            });
        }
    }
    YAML::Emitter out;
    out << YAML::BeginMap
        << YAML::Key << "site-name" << YAML::Value << site_name
        << YAML::Key << "site-url" << YAML::Value << site_url
        << YAML::Key << "site-description" << YAML::Value << site_description
        << YAML::Key << "ip" << YAML::Value << ip
        << YAML::Key << "port" << YAML::Value << port
        << YAML::Key << "theme" << YAML::Value << theme
        << YAML::Key << "users" << YAML::Value << YAML::Node(users)
        << YAML::EndMap;
    ofstream fs(gomibako.get_config_filename());
    if (!fs) {
        return crow::response("failed");
    }
    fs << out.c_str();
    fs.close();
    return crow::response("success");
}

void ErrorHandler::after_handle(crow::request &req, crow::response &res, ErrorHandler::context &ctx) {
    if (res.code == 200) {
        return;
    }
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<Theme> theme = gomibako.get_theme();
    if (theme->get_configuration().error_codes.count(res.code)) {
        ostringstream out;
        theme->render_error(out, res.code, gomibako.get_site_information(), gomibako.get_url_maker());
        res.body = out.str();
    }
}

void BasicAuth::before_handle(crow::request &req, crow::response &res, BasicAuth::context &ctx) {
    if (req.url.size() < 6 || req.url.compare(0, 6, "/admin") != 0) {
        return;
    }
    const string &header = req.get_header_value("Authorization");
    if (header.length() < 7 || header.compare(0, 6, "Basic ") != 0) {
        res.add_header("WWW-Authenticate", "Basic");
        res.code = 401;
        res.end();
        return;
    }
    string decoded;
    if (!Base64::Decode(header.substr(6), &decoded)) {
        res.add_header("WWW-Authenticate", "Basic");
        res.code = 401;
        res.end();
        return;
    }
    size_t pos = decoded.find(':');
    if (pos == string::npos || pos == (decoded.length() - 1)) {
        res.add_header("WWW-Authenticate", "Basic");
        res.code = 401;
        res.end();
        return;
    }
    const string &username = decoded.substr(0, pos);
    const string &password = decoded.substr(pos + 1);
    string password_hash;
    picosha2::hash256_hex_string(password, password_hash);
    const map<string, string> &users = Gomibako::get_instance().get_users();
    auto &&it = users.find(username);
    if (it == users.end() || it->second != password_hash) {
        res.add_header("WWW-Authenticate", "Basic");
        res.code = 401;
        res.end();
        return;
    }
}

StaticHandler::StaticHandler(const string &directory_, const map<string, string> &files_) :
    directory(directory_), files(files_) {}

crow::response StaticHandler::handle(const string &filename) {
    auto &&it = this->files.find(filename);
    if (it == this->files.end()) {
        it = this->files.find("*");
        if (it == this->files.end()) {
            return crow::response(404);
        }
    }
    string filename_simplified;
    if (!simplify_path(filename, filename_simplified)) {
        return crow::response(403);
    }
    const string &mime_type = it->second == "auto" ? get_mime_type(filename_simplified) : it->second;
    const string &path = this->directory + "/" + filename_simplified;
    ifstream fs(path, ios::binary);
    if (!fs) {
        return crow::response(404);
    }
    size_t size = fs.tellg();
    fs.seekg(0, ios::end);
    size = (size_t)fs.tellg() - size;
    fs.seekg(0, ios::beg);
    char *buf = new char[size];
    fs.read(buf, size);
    fs.close();
    string out(buf, size);
    delete[] buf;
    crow::response response(out);
    response.set_header("Content-Type", mime_type);
    return response;
}

string StaticHandler::get_mime_type(const string &filename) {
    static const map<string, string> MIMEType = {
        {"txt", "text/plain"},
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"xml", "application/xml"},
        {"json", "application/json"},
        {"ttf", "font/ttf"},
        {"eot", "application/vnd.ms-fontobject"},
        {"woff", "font/woff"},
        {"woff2", "font/woff2"}
    };
    size_t pos = filename.rfind('.');
    if (pos == string::npos || pos == filename.length() - 1) {
        return "application/octet-stream";
    }
    const string &ext = filename.substr(pos + 1);
    auto &&it = MIMEType.find(ext);
    if (it == MIMEType.cend()) {
        return "application/octet-stream";
    }
    return it->second;
}

bool StaticHandler::simplify_path(const string &path, string &out_) {
    vector<string> parts;
    ostringstream part, out;
    int state = 1;
    for (size_t i = 0; i <= path.length(); ++i) {
        switch (state) {
            case 1:
                if (path[i] == '/' || path[i] == '\0') {
                    state = 2;
                    const string &part_str = part.str();
                    if (part_str == "..") {
                        if (parts.size()) {
                            parts.pop_back();
                        } else {
                            return false;
                        }
                    } else if (part_str != "." && part_str != "") {
                        parts.push_back(part_str);
                    }
                } else {
                    part << path[i];
                }
                break;
            case 2:
                if (path[i] != '/') {
                    state = 1;
                    part.str("");
                    part << path[i];
                }
                break;
        }
    }
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i == 0) {
            out << parts[i];
        } else {
            out << "/" << parts[i];
        }
    }
    out_ = out.str();
    return true;
}