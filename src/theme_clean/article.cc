#include <string>
#include <sstream>
#include <set>
#include <iomanip>
#include <ctime>
#include "theme_api.h"
#include "layout.hpp"

using namespace std;
using namespace gomibako;

extern std::string comment_html;

extern"C" void render_article(ostringstream &out, const ArticleMetadata &metadata, const string &content,
                              const SiteInformation &site_information, shared_ptr<URLMaker> url_maker) {
    ostringstream title;
    title << metadata.title << " | " << site_information.name;
    header(out, site_information, url_maker, title.str());
    out <<
R"(<div class="post">
    <h1 id="title">)" << metadata.title << R"(</h1>
    <div class="date">)" << 
    put_time(localtime(&metadata.timestamp), "%b %d, %Y") << R"(
    </div>
    <article>)" <<
    content << R"(
    </article>)";
    if (metadata.tags.size() != 0) {
        out <<
    R"(<div class="tags">
        Tagged in :)";
        for (auto &&tag : metadata.tags) {
            out << R"(<a href=")" << url_maker->url_tag(tag) << "\">" << tag << "</a>";
        }
        out << "</div>";
    }
    out << comment_html;
    out << "</div>";
    footer(out);
}