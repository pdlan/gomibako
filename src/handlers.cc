#include <string>
#include <vector>
#include <sstream>
#include <crow.h>
#include "util/base64.h"
#include "util/picosha2.h"
#include "gomibako.h"
#include "handlers.h"
#include "url.h"

using namespace std;
using namespace gomibako;

crow::response handler_article(const std::string &id_encoded) {
    const std::string &id = urldecode(id_encoded);
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

crow::response handler_custom_page(const std::string &id_encoded) {
    const std::string &id = urldecode(id_encoded);
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

crow::response handler_tag(const std::string &tag_encoded, int page) {
    const std::string &tag = urldecode(tag_encoded);
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

crow::response handler_admin() {
    return crow::response("test");
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

StaticHandler::StaticHandler(const std::string &_directory, const std::map<std::string, std::string> &_files) :
    directory(_directory), files(_files) {}

crow::response StaticHandler::handle(const string &filename) {
    auto it = this->files.find(filename);
    if (it == this->files.end()) {
        return crow::response(404);
    }
    const std::string &mime_type = it->second;
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