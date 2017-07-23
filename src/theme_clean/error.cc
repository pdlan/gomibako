#include <string>
#include <sstream>
#include <set>
#include <ctime>
#include "theme_api.h"
#include "layout.hpp"

using namespace std;
using namespace gomibako;

extern"C" void render_error(ostringstream &out, int code, const SiteInformation &site_information,
                            shared_ptr<URLMaker> url_maker) {
    header(out, site_information, url_maker, "404 | " + site_information.name);
    out << "<h1>Oops, Page Not Found.</h1>";
    footer(out);
}