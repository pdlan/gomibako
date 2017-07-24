#include <string>
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <memory>
#include <ctime>
#include <cstdio>
#include <yaml-cpp/yaml.h>
#include <crow.h>
#include "util/yaml.h"
#include "gomibako.h"
#include "article.h"

using namespace gomibako;

ArticleManager::ArticleManager(const std::string &articles_path) {
    this->metadata_path = articles_path + "/metadata.yaml";
    this->content_path = articles_path + "/content/";
}

bool ArticleManager::load_metadata() {
    using namespace std;
    YAML::Node root;
    try {
        root = YAML::LoadFile(this->metadata_path);
    } catch (const YAML::BadFile &e) {
        return false;
    }
    if (!root.IsSequence()) {
        return false;
    }
    for (YAML::const_iterator it = root.begin(); it != root.end(); ++it) {
        ArticleMetadata metadata = it->as<ArticleMetadata>();
        this->id_metadata_map[metadata.id] = metadata;
        this->timestamp_id_pairs.push_back(std::make_pair(metadata.timestamp, metadata.id));
    }
    sort_metadata();
    this->tags.clear();
    for (auto &&i : this->id_metadata_map) {
        for (auto &&tag : i.second.tags) {
            if (this->tags.count(tag)) {
                ++this->tags[tag];
            } else {
                this->tags[tag] = 1;
            }
        }
    }
    return true;
}

void ArticleManager::sort_metadata() {
    typedef std::pair<time_t, std::string> T;
    std::sort(this->timestamp_id_pairs.begin(),
              this->timestamp_id_pairs.end(),
              [] (const T &a, const T &b) {return a.first > b.first;});
}

bool ArticleManager::get_metadata(const std::string &id, ArticleMetadata &metadata) const {
    auto it = this->id_metadata_map.find(id);
    if (it == this->id_metadata_map.end()) {
        return false;
    } else {
        metadata = it->second;
        return true;
    }
}

bool ArticleManager::get_metadata(const std::vector<std::string> &ids,
                                  std::vector<ArticleMetadata> &metadata) const {
    metadata.resize(ids.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        auto it = this->id_metadata_map.find(ids[i]);
        if (it == this->id_metadata_map.end()) {
            return false;
        }
        metadata[i] = it->second;
    }
    return true;
}

bool ArticleManager::get_content(const std::string &id, std::ostringstream &out) const {
    using namespace std;
    auto it = this->id_metadata_map.find(id);
    if (it == this->id_metadata_map.end()) {
        return false;
    }
    ifstream fs(this->content_path + it->second.filename);
    if (!fs) {
        return false;
    }
    out << fs.rdbuf();
    fs.close();
    return true;
}

bool ArticleManager::get_content(const std::vector<std::string> &ids,
                                 std::vector<std::ostringstream> &out) const {
    using namespace std;
    out.resize(ids.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        auto it = this->id_metadata_map.find(ids[i]);
        if (it == this->id_metadata_map.end()) {
            return false;
        }
        ifstream fs(this->content_path + it->second.filename);
        if (!fs) {
            return false;
        }
        out[i] << fs.rdbuf();
        fs.close();
    }
    return true;
}

bool ArticleManager::save_metadata() {
    YAML::Emitter out;
    out << YAML::BeginSeq;
    for (auto &&i : this->id_metadata_map) {
        YAML::Node metadata(i.second);
        out << metadata;
    }
    out << YAML::EndSeq;
    if (!out.good()) {
        return false;
    }
    std::ofstream fs(this->metadata_path);
    if (!fs) {
        return false;
    }
    fs << out.c_str();
    fs.close();
    return true;
}

std::string ArticleManager::add_article(const std::string &title, const std::string &content,
                                        time_t timestamp, const std::set<std::string> &tags) {
    std::string id = generate_id(title);
    std::string filename = generate_filename(id);
    ArticleMetadata metadata;
    metadata.id = id;
    metadata.title = title;
    metadata.tags = tags;
    metadata.timestamp = timestamp;
    metadata.filename = filename;
    std::ofstream fs(this->content_path + filename);
    if (!fs) {
        return "";
    }
    fs << content;
    fs.close();
    this->id_metadata_map[id] = metadata;
    this->timestamp_id_pairs.push_back(std::make_pair(metadata.timestamp, id));
    sort_metadata();
    save_metadata();
    return id;
}

bool ArticleManager::delete_article(const std::string &id, bool delete_file) {
    auto it = this->id_metadata_map.find(id);
    if (it == this->id_metadata_map.end()) {
        return false;
    }
    std::string filename = this->content_path + it->second.filename;
    this->id_metadata_map.erase(it);
    for (auto it = this->timestamp_id_pairs.begin();
         it != this->timestamp_id_pairs.end(); ++it) {
        if (it->second == id) {
            this->timestamp_id_pairs.erase(it);
            break;
        }
    }
    save_metadata();
    if (delete_file) {
        remove(filename.c_str());
    }
    return true;
}

bool ArticleManager::edit_article(const std::string &id, const std::string &title, time_t timestamp, 
                                  const std::set<std::string> &tags, const std::string &content) {
    auto it = this->id_metadata_map.find(id);
    if (it == this->id_metadata_map.end()) {
        return false;
    }
    auto it2 = std::find(this->timestamp_id_pairs.begin(),
                         this->timestamp_id_pairs.end(),
                         std::make_pair(it->second.timestamp, it->second.id));
    std::ofstream fs(it->second.filename);
    if (!fs) {
        return false;
    }
    fs << content;
    fs.close();
    it->second.title = title;
    it->second.timestamp = timestamp;
    it->second.tags = tags;
    it2->first = timestamp;
    sort_metadata();
    return save_metadata();
}

void ArticleManager::apply_filter(const Filter &filter, std::vector<std::string> &ids) const {
    TimeIDVector out;
    filter(this->timestamp_id_pairs, this->id_metadata_map, out);
    ids.resize(out.size());
    for (size_t i = 0; i < out.size(); ++i) {
        ids[i] = out[i].second;
    }
}

void ArticleManager::apply_filters(const std::vector<Filter> &filters,
                                   std::vector<std::string> &ids) const {
    if (filters.size() == 0) {
        apply_filter(filters[0], ids);
        return;
    }
    TimeIDVector out;
    filters[0](this->timestamp_id_pairs, this->id_metadata_map, out);
    for (size_t i = 1; i < filters.size(); ++i) {
        TimeIDVector out2;
        filters[i](out, this->id_metadata_map, out2);
        out = out2;
    }
    ids.resize(out.size());
    for (size_t i = 0; i < out.size(); ++i) {
        ids[i] = out[i].second;
    }
}

std::string ArticleManager::generate_id(const std::string &title) {
    if (this->id_metadata_map.count(title) == 0) {
        return title;
    }
    std::ostringstream ss;
    for (int i = 1; ; ++i) {
        ss.str("");
        ss << title << "-" << i;
        if (this->id_metadata_map.count(ss.str()) == 0) {
            break;
        }
    }
    return ss.str();
}

std::string ArticleManager::generate_filename(const std::string &id) {
    return crow::utility::base64encode_urlsafe(id.c_str(), id.length()) + ".txt";
}

PageManager::PageManager(const std::string &_pages_path) : pages_path(_pages_path) {}

bool PageManager::load_pages() {
    using namespace std;
    YAML::Node root;
    try {
        root = YAML::LoadFile(this->pages_path);
    } catch (const YAML::BadFile &e) {
        return false;
    }
    if (!root.IsSequence()) {
        return false;
    }
    for (YAML::const_iterator it = root.begin(); it != root.end(); ++it) {
        CustomPage page;
        if (!extract_yaml_map(*it,
            make_pair("order", &page.order),
            make_pair("id", &page.id),
            make_pair("title", &page.title),
            make_pair("content", &page.content)
            )) {
            return false;
        }
        this->pages.push_back(page);
    }
    sort_pages();
    return true;
}

void PageManager::sort_pages() {
    std::sort(this->pages.begin(), this->pages.end(), [](const CustomPage &a, const CustomPage &b) {
        return a.order < b.order;
    });
}

bool PageManager::get_page(const std::string &id, CustomPage &page) const {
    auto it = this->pages.begin();
    while (it != this->pages.end()) {
        if (it->id == id) {
            page = *it;
            return true;
        }
    }
    return false;
}

bool PageManager::edit_page(const std::string &id, int order, const std::string &title,
                            const std::string &content) {
    auto it = std::find_if(this->pages.begin(), this->pages.end(), [&id](const CustomPage &p) {
        return p.id == id;
    });
    if (it == this->pages.end()) {
        return false;
    }
    it->title = title;
    it->content = content;
    return save_pages();
}

bool PageManager::save_pages() {
    YAML::Emitter out;
    out << YAML::BeginSeq;
    for (auto &&i : this->pages) {
        out << YAML::BeginMap
            << YAML::Key << "id" << YAML::Value << i.id
            << YAML::Key << "order" << YAML::Value << i.order
            << YAML::Key << "title" << YAML::Value << i.title
            << YAML::Key << "content" << YAML::Value << i.content
            << YAML::EndMap;
    }
    out << YAML::EndSeq;
    if (!out.good()) {
        return false;
    }
    std::ofstream fs(this->pages_path);
    if (!fs) {
        return false;
    }
    fs << out.c_str();
    fs.close();
    return true;
}

bool PageManager::delete_page(const std::string &id) {
    auto it = std::find_if(this->pages.begin(), this->pages.end(), [&id](const CustomPage &p) {
        return p.id == id;
    });
    if (it == this->pages.end()) {
        return false;
    }
    this->pages.erase(it);
    this->sort_pages();
    return save_pages();
}