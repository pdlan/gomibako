#ifndef GOMIBAKO_INCLUDE_HANDLERS_H
#define GOMIBAKO_INCLUDE_HANDLERS_H
#include <string>
#include <sstream>
#include <set>
#include <crow.h>

crow::response handler_article(const std::string &id_encoded);
crow::response handler_custom_page(const std::string &id_encoded);
crow::response handler_page(int page);
crow::response handler_archives(int page);
crow::response handler_tag(const std::string &tag_encoded, int page);
crow::response handler_tags();
crow::response handler_feed();
crow::response handler_admin();
crow::response handler_admin_article(int page);
crow::response handler_admin_article_new(const crow::request &req);
crow::response handler_admin_article_edit(const crow::request &req);
crow::response handler_admin_article_delete(const std::string &id);
crow::response handler_admin_article_json(const std::string &id_encoded);
crow::response handler_admin_article_move(const std::string &id_encoded);
crow::response handler_admin_draft(int page);
crow::response handler_admin_draft_new(const crow::request &req);
crow::response handler_admin_draft_edit(const crow::request &req);
crow::response handler_admin_draft_delete(const std::string &id_encoded);
crow::response handler_admin_draft_json(const std::string &id_encoded);
crow::response handler_admin_draft_publish(const crow::request &req);
crow::response handler_admin_page();
crow::response handler_admin_page_new(const crow::request &req);
crow::response handler_admin_page_edit(const crow::request &req);
crow::response handler_admin_page_delete(const std::string &id_encoded);
crow::response handler_admin_page_json(const std::string &id_encoded);
crow::response handler_admin_config();
crow::response handler_admin_config_edit(const crow::request &req);

struct ErrorHandler {
    struct context {};
    void before_handle(crow::request &req, crow::response &res, context &ctx) {}
    void after_handle(crow::request &req, crow::response &res, context &ctx);
};

struct BasicAuth {
    struct context {};
    void before_handle(crow::request &req, crow::response &res, context &ctx);
    void after_handle(crow::request &req, crow::response &res, context &ctx) {}
};

class StaticHandler {
public:
    StaticHandler(const std::string &directory, const std::map<std::string, std::string> &files);
    crow::response handle(const std::string &filename);
private:
    std::string directory;
    std::map<std::string, std::string> files;
    static std::string get_mime_type(const std::string &filename);
    static bool simplify_path(const std::string &path, std::string &out);
};
#endif