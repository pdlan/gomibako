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
crow::response handler_admin();

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
};
#endif