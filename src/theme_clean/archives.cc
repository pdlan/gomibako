#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "theme_api.h"
#include "layout.hpp"

using namespace std;
using namespace gomibako;

extern"C" void render_archives(ostringstream &out, const vector<ArticleMetadata> &metadata,
                               int page, int pages, const SiteInformation &site_information,
                               shared_ptr<URLMaker> url_maker) {
    header(out, site_information, url_maker, "Archives | " + site_information.name);
    int prev_year;
    for (size_t i = 0; i < metadata.size(); ++i) {
        tm *_tm = localtime(&metadata[i].timestamp);
        int year = _tm->tm_year + 1900;
        if (i == 0) {
            prev_year = year;
            out << "<h2>" << year << "</h2>\n<ol id=\"posts\">";
        }
        if (year != prev_year) {
            out << "</ol>\n<h2>" << year << "</h2>\n<ol id=\"posts\">";
            prev_year = year;
        }
        out << R"(<li><span class="meta">)" << put_time(_tm, "%b %d") << R"(</span><a href=")"
            << url_maker->url_article(metadata[i].id) << "\">" << metadata[i].title << "</a></li>\n";
        if (i == (metadata.size() - 1)) {
            out << "</ol>";
        }
    }
    out <<R"(    <div id="pagination">)";
    if (page != 1) {
        out << R"(<a id="prev" href=")" << url_maker->url_archives(page - 1) << "\">Prev</a>";
    }

    out << "<span>Page " << page << " of " << pages << "</span>";

    if (page != pages) {
        out << R"(<a id="next" href=")" << url_maker->url_archives(page + 1) << "\">Next</a>";
    }
    out << "</div>";
    footer(out);
}