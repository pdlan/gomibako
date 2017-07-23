#ifndef GOMIBAKO_INCLUDE_URL_H
#define GOMIBAKO_INCLUDE_URL_H
#include <string>

std::string urlencode(const char *data, size_t length);
std::string urldecode(const std::string &data);

class URLMaker {
public:
    URLMaker(const std::string &_site_url) : site_url(_site_url) {}
    virtual std::string url_article(const std::string &id);
    virtual std::string url_page(int page);
    virtual std::string url_tag(const std::string &tag, int page = 1);
    virtual std::string url_index();
    virtual std::string url_archives(int page = 1);
    virtual std::string url_static(const std::string &path);
    virtual std::string url_custom_page(const std::string &id);
private:
    std::string site_url;
};
#endif