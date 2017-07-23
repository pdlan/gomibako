#ifndef GOMIBAKO_INCLUDE_ARTICLE_H
#define GOMIBAKO_INCLUDE_ARTICLE_H
#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include <set>
#include <functional>
#include <ctime>
#include <yaml-cpp/yaml.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace gomibako {
struct ArticleMetadata {
    std::string id;
    std::string title;
    std::string filename;
    time_t timestamp;
    std::set<std::string> tags;
};

struct CustomPage {
    int order;
    std::string title;
    std::string id;
    std::string content;
};

typedef std::vector<std::pair<time_t, std::string>> TimeIDVector;
typedef std::map<std::string, ArticleMetadata> IDMetadataMap;
typedef std::function<void (const TimeIDVector &, const IDMetadataMap &, TimeIDVector &)> Filter;

class ArticleManager {
public:
    ArticleManager(const std::string &articles_path);
    bool load_metadata();
    bool save_metadata();
    bool get_metadata(const std::string &id, ArticleMetadata &metadata) const;
    bool get_metadata(const std::vector<std::string> &ids,
                      std::vector<ArticleMetadata> &metadata) const;
    bool get_content(const std::string &id, std::ostringstream &out) const;
    bool get_content(const std::vector<std::string> &ids,
                     std::vector<std::ostringstream> &out) const;
    std::string add_article(const std::string &title, const std::string &content,
                            time_t timestamp, const std::set<std::string> &tags);
    bool delete_article(const std::string &id, bool delete_file = false);
    void sort_metadata();
    void apply_filter(const Filter &filter, std::vector<std::string> &ids) const;
    void apply_filters(const std::vector<Filter> &filters, std::vector<std::string> &ids) const;
private:
    TimeIDVector timestamp_id_pairs;
    IDMetadataMap id_metadata_map;
    std::string metadata_path, content_path;
    std::string generate_id(const std::string &title);
    std::string generate_filename(const std::string &id);
};

class PageManager {
public:
    PageManager(const std::string &pages_path);
    bool load_pages();
    bool save_pages();
    void sort_pages();
    bool delete_page(const std::string &id);
    inline std::vector<CustomPage> * const get_pages() {return &this->pages;};
    bool get_page(const std::string &id, CustomPage &page) const;
private:
    std::string pages_path;
    std::vector<CustomPage> pages;
};
}

namespace YAML {
template<>
struct convert<gomibako::ArticleMetadata> {
    static Node encode(const gomibako::ArticleMetadata &rhs) {
        using namespace boost::posix_time;
        Node node;
        node["id"] = rhs.id;
        node["title"] = rhs.title;
        node["filename"] = rhs.filename;
        Node tags;
        for (const std::string &i : rhs.tags) {
            tags.push_back(i);
        }
        node["tags"] = tags;
        node["time"] = to_iso_string(from_time_t(rhs.timestamp));
        return node;
    }
    
    static bool decode(const Node &node, gomibako::ArticleMetadata &rhs) {
        using namespace boost::posix_time;
        if (!node.IsMap()) {
            return false;
        }
        if (!(node["id"] && node["id"].IsScalar() &&
              node["title"] && node["title"].IsScalar() &&
              node["filename"] && node["filename"].IsScalar() &&
              node["time"] && node["time"].IsScalar() &&
              node["tags"] && node["tags"].IsSequence())) {
            return false;
        }
        rhs.id = node["id"].as<std::string>();
        rhs.title = node["title"].as<std::string>();
        rhs.filename = node["filename"].as<std::string>();
        Node tags = node["tags"];
        for (const_iterator it = tags.begin(); it != tags.end(); ++it) {
            if (!it->IsScalar()) {
                return false;
            }
            rhs.tags.insert(it->as<std::string>());
        }
        ptime pt = from_iso_string(node["time"].as<std::string>());
        if (pt.is_special()) {
            return false;
        }
        rhs.timestamp = to_time_t(pt);
        return true;
    }
};
}
#endif