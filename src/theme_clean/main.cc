#include <string>
#include <sstream>
#include <fstream>
#include "theme_api.h"

using namespace std;
using namespace gomibako;

std::string comment_html;

extern"C" bool init(ThemeConfiguration &config) {
    config.articles_per_page = 20;
    config.articles_per_page_tag = 20;
    config.articles_per_page_archives = 20;
    config.static_directory = "theme";
    config.static_files  = {
        {"clean.css", "text／css"}
    };
    ifstream fs("comment.html");
    if (fs) {
        ostringstream ss;
        ss << fs.rdbuf();
        comment_html = ss.str();
        fs.close();
    }
    return true;
}