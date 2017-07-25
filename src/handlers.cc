#include <string>
#include <vector>
#include <sstream>
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
    vector<ostringstream> content;
    article_manager->get_metadata(ids, metadata);
    article_manager->get_content(ids, content);
    ostringstream out;
    gomibako.get_theme()->render_page(out, metadata, content, page, pages,
                                      gomibako.get_site_information(), gomibako.get_url_maker());
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
    gomibako.get_theme()->render_tag(out, metadata, content, tag, page, pages,
                                     gomibako.get_site_information(), gomibako.get_url_maker());
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

crow::response handler_admin() {
    crow::json::wvalue ctx;
    ctx["title"] = "Home | Dashboard";
    return crow::response(crow::mustache::load("index.html").render(ctx));
}

crow::response handler_admin_article_draft(shared_ptr<ArticleManager> manager, 
                                            const char *tpl, int page) {
    if (page == 0) {
        return crow::response(404);
    }
    vector<string> ids;
    int pages;
    manager->apply_filter(Pager(10).get_filter(page, pages), ids);
    if (page > pages && pages != 0) {
        return crow::response(404);
    }
    vector<ArticleMetadata> metadata;
    manager->get_metadata(ids, metadata);
    crow::json::wvalue ctx;
    ctx["editor"] = true;
    if (strcmp(tpl, "article.html") == 0) {
        ctx["article_js"] = true;
    } else {
        ctx["draft_js"] = true;
    }
    ctx["title"] = "Article | Dashboard";
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
    set<string> tags;
    for (auto &&tag : tags_vector) {
        if (!tags.count(tag)) {
            tags.insert(tag);
        }
    }
    if (manager->edit_article(
        id,
        title,
        time(nullptr),
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
    draft_manager->apply_filter(
        [&] (const TimeIDVector &ids, const IDMetadataMap &metadata, TimeIDVector &_out) {
            return get_pagination(ids, metadata, _out, new_id, 10, page, pages);
        },
    empty);
    out << "/admin/draft/page/" << page << "/";
    return crow::response(out.str());
}

crow::response handler_admin_article(int page) {
    Gomibako &gomibako = Gomibako::get_instance();
    return handler_admin_article_draft(gomibako.get_article_manager(), "article.html", page);
}

crow::response handler_admin_article_new(const crow::request &req) {
    return handler_admin_article_draft_new(Gomibako::get_instance().get_article_manager(), req);
}

crow::response handler_admin_article_edit(const crow::request &req) {
    return handler_admin_article_draft_edit(Gomibako::get_instance().get_article_manager(), req);
}

crow::response handler_admin_article_delete(const string &id_encoded) {
    return handler_admin_article_draft_delete(Gomibako::get_instance().get_article_manager(), id_encoded);
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
    article_manager->apply_filter(
        [&] (const TimeIDVector &ids, const IDMetadataMap &metadata, TimeIDVector &_out) {
            return get_pagination(ids, metadata, _out, new_id, 10, page, pages);
        },
    empty);
    out << "/admin/article/page/" << page << "/";
    return crow::response(out.str());
}

void ErrorHandler::after_handle(crow::request &req, crow::response &res, ErrorHandler::context &ctx) {
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<Theme> theme = gomibako.get_theme();
    const set<int> &codes = theme->get_configuration().error_codes;
    if (res.code != 200 && codes.count(res.code)) {
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
    if (header.empty()) {
        res.add_header("WWW-Authenticate", "Basic");
        res.code = 401;
        res.end();
        return;
    }
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
    string username = decoded.substr(0, pos);
    string password = decoded.substr(pos + 1);
    string password_hash;
    picosha2::hash256_hex_string(password, password_hash);
    const auto &users = Gomibako::get_instance().get_users();
    auto it = users.find(username);
    if (it == users.end() || it->second != password_hash) {
        res.add_header("WWW-Authenticate", "Basic");
        res.code = 401;
        res.end();
        return;
    }
}

StaticHandler::StaticHandler(const string &_directory, const map<string, string> &_files) :
    directory(_directory), files(_files) {}

crow::response StaticHandler::handle(const string &filename) {
    auto it = this->files.find(filename);
    if (it == this->files.end()) {
        return crow::response(404);
    }
    const string &mime_type = it->second;
    ostringstream path;
    path << this->directory << "/" << filename;
    ifstream fs(path.str(), ios::binary);
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