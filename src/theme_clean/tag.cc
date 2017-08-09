#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "theme_api.h"
#include "layout.hpp"

using namespace std;
using namespace gomibako;

extern"C" void render_tag(ostringstream &out, const vector<ArticleMetadata> &metadata,
                          const std::function<bool (const std::string &, std::ostringstream &)> &get_content,
                          const std::string &tag,
                          int page, int pages, const SiteInformation &site_information,
                          shared_ptr<URLMaker> url_maker) {
    ostringstream title;
    title << tag << " | " << site_information.name;
    header(out, site_information, url_maker, title.str());
    out << "<h2>Tag: " << tag << "</h2>"
        << R"(<ol id="posts">)";
    for (size_t i = 0; i < metadata.size(); ++i) {
        out <<
R"(
    <li>
        <span class="meta">)" << put_time(localtime(&metadata[i].timestamp), "%Y-%m-%d") <<
R"(</span> <a href=")" << url_maker->url_article(metadata[i].id) << "\">" << metadata[i].title << R"(</a>
    </li>)";
    }
    out <<
R"(</ol>

<div id="pagination">)";
    if (page != 1) {
        out << R"(<a id="prev" href=")" << url_maker->url_tag(tag, page - 1) << "\">Prev</a>";
    }

    out << "<span>Page " << page << " of " << pages << "</span>";

    if (page != pages) {
        out << R"(<a id="next" href=")" << url_maker->url_tag(tag, page + 1) << "\">Next</a>";
    }
    out << "</div>";
    footer(out);
}