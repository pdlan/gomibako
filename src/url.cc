#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "url.h"

using namespace gomibako;

std::string gomibako::urlencode(const char *data, size_t length) {
    using namespace std;
    ostringstream os;
    for (size_t i = 0; i < length; ++i) {
        switch (data[i]) {
            case 'A': os << 'A'; break;
            case 'B': os << 'B'; break;
            case 'C': os << 'C'; break;
            case 'D': os << 'D'; break;
            case 'E': os << 'E'; break;
            case 'F': os << 'F'; break;
            case 'G': os << 'G'; break;
            case 'H': os << 'H'; break;
            case 'I': os << 'I'; break;
            case 'J': os << 'J'; break;
            case 'K': os << 'K'; break;
            case 'L': os << 'L'; break;
            case 'M': os << 'M'; break;
            case 'N': os << 'N'; break;
            case 'O': os << 'O'; break;
            case 'P': os << 'P'; break;
            case 'Q': os << 'Q'; break;
            case 'R': os << 'R'; break;
            case 'S': os << 'S'; break;
            case 'T': os << 'T'; break;
            case 'U': os << 'U'; break;
            case 'V': os << 'V'; break;
            case 'W': os << 'W'; break;
            case 'X': os << 'X'; break;
            case 'Y': os << 'Y'; break;
            case 'Z': os << 'Z'; break;
            case 'a': os << 'a'; break;
            case 'b': os << 'b'; break;
            case 'c': os << 'c'; break;
            case 'd': os << 'd'; break;
            case 'e': os << 'e'; break;
            case 'f': os << 'f'; break;
            case 'g': os << 'g'; break;
            case 'h': os << 'h'; break;
            case 'i': os << 'i'; break;
            case 'j': os << 'j'; break;
            case 'k': os << 'k'; break;
            case 'l': os << 'l'; break;
            case 'm': os << 'm'; break;
            case 'n': os << 'n'; break;
            case 'o': os << 'o'; break;
            case 'p': os << 'p'; break;
            case 'q': os << 'q'; break;
            case 'r': os << 'r'; break;
            case 's': os << 's'; break;
            case 't': os << 't'; break;
            case 'u': os << 'u'; break;
            case 'v': os << 'v'; break;
            case 'w': os << 'w'; break;
            case 'x': os << 'x'; break;
            case 'y': os << 'y'; break;
            case 'z': os << 'z'; break;
            case '0': os << '0'; break;
            case '1': os << '1'; break;
            case '2': os << '2'; break;
            case '3': os << '3'; break;
            case '4': os << '4'; break;
            case '5': os << '5'; break;
            case '6': os << '6'; break;
            case '7': os << '7'; break;
            case '8': os << '8'; break;
            case '9': os << '9'; break;
            case '-': os << '-'; break;
            case '_': os << '_'; break;
            case '.': os << '.'; break;
            case '~': os << '~'; break;
            default: os << '%' << setfill('0') << setw(2) << uppercase << hex
                        << (unsigned int)(reinterpret_cast<const unsigned char *>(data)[i]);
        }
    }
    return os.str();
}

inline char get_digit(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c - 'A' + 10;
}

std::string gomibako::urldecode(const std::string &data) {
    using namespace std;
    ostringstream os;
    for (size_t i = 0; i < data.length(); ++i) {
        if (data[i] == '%') {
            if (i > data.length() - 2) {
                break;
            }
            char digit1 = get_digit(data[i+1]);
            char digit2 = get_digit(data[i+2]);
            os << char(digit1 * 16 + digit2);
            i += 2;
        } else {
            os << data[i];
        }
    }
    return os.str();
}

std::string SimpleURLMaker::url_article(const std::string &id) {
    using namespace std;
    ostringstream os;
    os << site_url << "/article/" << urlencode(id.c_str(), id.length());
    return os.str(); 
}

std::string SimpleURLMaker::url_page(int page) {
    using namespace std;
    ostringstream os;
    if (page != 1) {
        os << site_url << "/page/" << page;
    } else {
        os << site_url << "/";
    }
    return os.str(); 
}

std::string SimpleURLMaker::url_tag(const std::string &tag, int page) {
    using namespace std;
    ostringstream os;
    os << site_url << "/tag/" << urlencode(tag.c_str(), tag.length()) << "/";
    if (page != 1) {
        os << "page/" << page;
    }
    return os.str(); 
}

std::string SimpleURLMaker::url_index() {
    return this->site_url + "/";
}

std::string SimpleURLMaker::url_archives(int page) {
    using namespace std;
    ostringstream os;
    os << site_url << "/archives/";
    if (page != 1) {
        os << "page/" << page;
    }
    return os.str(); 
}

std::string SimpleURLMaker::url_static(const std::string &path) {
    using namespace std;
    ostringstream os;
    os << site_url << "/static/" << path;
    return os.str(); 
}

std::string SimpleURLMaker::url_custom_page(const std::string &id) {
    using namespace std;
    ostringstream os;
    os << site_url << "/page/c" << id;
    return os.str(); 
}

std::string SimpleURLMaker::url_tags() {
    return site_url + "/tags";
}