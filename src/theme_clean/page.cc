#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
#include <ctime>
#include "theme_api.h"
#include "layout.hpp"

using namespace std;
using namespace gomibako;

extern"C" void render_page(ostringstream &out, const vector<ArticleMetadata> &metadata,
                           const std::function<bool (const std::string &, std::ostringstream &)> &get_content,
                           int page, int pages, const SiteInformation &site_information,
                           shared_ptr<URLMaker> url_maker) {
    header(out, site_information, url_maker, site_information.name);
    out << R"(<ol id="posts">)";
    for (size_t i = 0; i < metadata.size(); ++i) {
        out <<
R"(
    <li>
        <span class="date">)" << put_time(localtime(&metadata[i].timestamp), "%b %d, %Y") <<
R"(</span> <a href=")" << url_maker->url_article(metadata[i].id) << "\">" << metadata[i].title << R"(</a>
    </li>)";
    }
    out <<
R"(</ol>

<div id="pagination">)";
    if (page != 1) {
        out << R"(<a id="prev" href=")" << url_maker->url_page(page - 1) << "\">Prev</a>";
    }

    out << "<span>Page " << page << " of " << pages << "</span>";

    if (page != pages) {
        out << R"(<a id="next" href=")" << url_maker->url_page(page + 1) << "\">Next</a>";
    }
    out << "</div>";
    footer(out);
}