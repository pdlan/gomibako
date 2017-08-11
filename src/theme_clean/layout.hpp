#ifndef GOMIBAKO_SRC_THEME_CLEAN_LAYOUT_HPP
#define GOMIBAKO_SRC_THEME_CLEAN_LAYOUT_HPP
#include <string>
#include <sstream>
#include "theme_api.h"

using namespace gomibako;

inline void header(std::ostringstream &out, const SiteInformation &site_information,
                   std::shared_ptr<URLMaker> url_maker, const std::string &title) {
    out <<
R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width" />
    <title>)" << title << R"(</title>
    <link rel="stylesheet" href=")" << url_maker->url_static("clean.css") << R"(" type="text/css" />
    <link rel="alternate" type="application/rss+xml" href=")" << url_maker->url_feed()<< R"(" title=")" << site_information.name << R"(" />
</head>
<body>
    <div id="wrapper" class="yue">
        <header id="header">
            <a href=")" << url_maker->url_index() << R"(" id="site_name">)" << site_information.name << R"(</a>
            <nav>
                <a href=")" << url_maker->url_archives() << R"(">Archives</a>)" << R"(
                <a href=")" << url_maker->url_tags() << R"(">Tags</a>)";
    for (auto &&page: *site_information.pages) {
        out << "<a href=\"" << url_maker->url_custom_page(page.id) << "\">" << page.title << "</a>";
    }
    out <<
R"(
            </nav>
        </header>
        <div id="content">
)";
}

inline void footer(std::ostringstream &out) {
    out <<
R"(</div>
        <footer id="footer">
            Powered by <a href="http://github.com/pdlan/gomibako">Gomibako</a>
            &
            Theme <a href="https://github.com/whtsky/catsup-theme-clean">Clean</a> by <a href="http://whouz.com">whtsky</a>
        </footer>
    </div>
</body>
</html>
)";
}
#endif