#ifndef GOMIBAKO_INCLUDE_ARTICLE_H
#define GOMIBAKO_INCLUDE_ARTICLE_H
#include <string>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <set>
#include <functional>
#include <ctime>
#include <cstring>
#include <yaml-cpp/yaml.h>

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
typedef std::unordered_map<std::string, ArticleMetadata> IDMetadataMap;
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
    bool edit_article(const std::string &id, const std::string &title, time_t timestamp, 
                      const std::set<std::string> &tags, const std::string &content);
    void sort_metadata();
    inline const std::map<std::string, int> & get_tags() {return this->tags;}
    void apply_filter(const Filter &filter, std::vector<std::string> &ids) const;
    void apply_filters(const std::vector<Filter> &filters, std::vector<std::string> &ids) const;
private:
    TimeIDVector timestamp_id_pairs;
    IDMetadataMap id_metadata_map;
    std::map<std::string, int> tags;
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
    std::string add_page(int order, const std::string &title, const std::string &content);
    inline std::vector<CustomPage> * const get_pages() {return &this->pages;};
    bool get_page(const std::string &id, CustomPage &page) const;
    bool edit_page(const std::string &id, int order, const std::string &title,
                   const std::string &content);
private:
    std::string pages_path;
    std::vector<CustomPage> pages;
    std::string generate_id(const std::string &title);
};
}

namespace YAML {
template<>
struct convert<gomibako::ArticleMetadata> {
    static Node encode(const gomibako::ArticleMetadata &rhs) {
        Node node;
        node["id"] = rhs.id;
        node["title"] = rhs.title;
        node["filename"] = rhs.filename;
        Node tags;
        for (const std::string &i : rhs.tags) {
            tags.push_back(i);
        }
        node["tags"] = tags;
        std::ostringstream os;
        os << std::put_time(localtime(&rhs.timestamp), "%Y%m%dT%H%M%S");
        node["time"] = os.str();
        return node;
    }
    
    static bool decode(const Node &node, gomibako::ArticleMetadata &rhs) {
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
        tm tmb;
        memset((void *)&tmb, 0, sizeof(tmb));
        std::istringstream is(node["time"].as<std::string>());
        is >> std::get_time(&tmb, "%Y%m%dT%H%M%S");
        if (!is) {
            return false;
        }
        rhs.timestamp = mktime(&tmb);
        return true;
    }
};
}
#endif