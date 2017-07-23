#include <string>
#include <sstream>
#include <fstream>
#include "theme_api.h"

using namespace std;
using namespace gomibako;

string comment_html;

extern"C" bool init(ThemeConfiguration &config) {
    config.articles_per_page = 20;
    config.articles_per_page_tag = 20;
    config.articles_per_page_archives = 20;
    config.static_directory = "assets";
    config.static_files  = {
        {"clean.css", "text/css"}
    };
    config.error_codes = {404};
    ifstream fs("comment.html");
    if (fs) {
        ostringstream ss;
        ss << fs.rdbuf();
        comment_html = ss.str();
        fs.close();
    }
    return true;
}