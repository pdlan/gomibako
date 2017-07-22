#ifndef GOMIBAKO_INCLUDE_HANDLERS_H
#define GOMIBAKO_INCLUDE_HANDLERS_H
#include <string>
#include <sstream>
#include <crow.h>


//crow::response handler_tag(const std::string &tag, int page);
crow::response handler_article(const std::string &id_encoded);
crow::response handler_page(int page);

class StaticHandler {
public:
    StaticHandler(const std::string &directory, const std::map<std::string, std::string> &files);
    crow::response handle(const std::string &filename);
private:
    std::string directory;
    std::map<std::string, std::string> files;
};
#endif