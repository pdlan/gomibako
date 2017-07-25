#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include "theme_api.h"
#include "layout.hpp"

using namespace std;
using namespace gomibako;

extern"C" void render_tags(ostringstream &out, const std::map<std::string, int> &tags,
                           const SiteInformation &site_information, shared_ptr<URLMaker> url_maker) {
    header(out, site_information, url_maker, "Tags | " + site_information.name);
    for (auto &&i : tags) {
        int size = round(log10(i.second) * 10 + 10);
        out << "<a style=\"padding:5px;font-size:" << size << "px;\" href=\"" 
            << url_maker->url_tag(i.first) << "\">" << i.first << " ("
            << i.second << ")</a>";
    }
    footer(out);
}