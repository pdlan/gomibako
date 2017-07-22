#include <string>
#include <vector>
#include <sstream>
#include <crow.h>
#include "gomibako.h"
#include "handlers.h"
#include "url.h"

using namespace std;
using namespace gomibako;

crow::response handler_article(const std::string &id_encoded) {
    Gomibako &gomibako = Gomibako::get_instance();
    shared_ptr<ArticleManager> article_manager = gomibako.get_article_manager();
    const std::string &id = urldecode(id_encoded);
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
    ostringstream os;
    for (auto &&i : metadata) {
        os << i.id << ", " << i.title << "," << i.timestamp << endl;
    }
    return crow::response(os.str());
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