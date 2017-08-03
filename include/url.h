#ifndef GOMIBAKO_INCLUDE_URL_H
#define GOMIBAKO_INCLUDE_URL_H
#include <string>

namespace gomibako {
std::string urlencode(const char *data, size_t length);
std::string urldecode(const std::string &data);

class URLMaker {
public:
    virtual std::string url_article(const std::string &id) = 0;
    virtual std::string url_page(int page) = 0;
    virtual std::string url_tag(const std::string &tag, int page = 1) = 0;
    virtual std::string url_index() = 0;
    virtual std::string url_archives(int page = 1) = 0;
    virtual std::string url_static(const std::string &path) = 0;
    virtual std::string url_custom_page(const std::string &id) = 0;
    virtual std::string url_tags() = 0;
};

class SimpleURLMaker : public URLMaker {
public:
    SimpleURLMaker(const std::string &_site_url) : site_url(_site_url) {}
    std::string url_article(const std::string &id);
    std::string url_page(int page);
    std::string url_tag(const std::string &tag, int page = 1);
    std::string url_index();
    std::string url_archives(int page = 1);
    std::string url_static(const std::string &path);
    std::string url_custom_page(const std::string &id);
    std::string url_tags();
private:
    std::string site_url;
};
}
#endif