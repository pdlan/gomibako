#ifndef GOMIBAKO_INCLUDE_FILTER_H
#define GOMIBAKO_INCLUDE_FILTER_H
#include <functional>
#include <map>
#include <vector>
#include "article.h"

namespace gomibako {
template<class T>
class CachedFilter {
public:
    CachedFilter(std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition) {
        this->condition = condition;
    }

    Filter get_filter(const T &arg) {
        return [=] (const TimeIDVector &ids, const IDMetadataMap &metadata, TimeIDVector &out) {
            auto it = this->cache.find(arg);
            if (it == this->cache.end()) {
                TimeIDVector *new_ids = new TimeIDVector;
                for (auto &&i : ids) {
                    if (this->condition(arg, metadata.at(i.second))) {
                        new_ids->push_back(i);
                    }
                }
                this->cache[arg] = new_ids;
                out = *new_ids;
            } else {
                out = *it->second;
            }
        };
    }

    void clear_cache() {
        for (auto &&i : this->cache) {
            delete i.second;
        }
    }

    ~CachedFilter() {
        clear_cache();
    }
private:
    std::map<T, TimeIDVector *> cache;
    std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition;
};

template<class T>
class DirectFilter {
public:
    DirectFilter(std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition) {
        this->condition = condition;
    }

    Filter get_filter(const T &arg) {
        return [=] (const TimeIDVector &ids, const IDMetadataMap &metadata, TimeIDVector &out) {
            for (auto &&i : ids) {
                if (this->condition(arg, metadata.at(i.second))) {
                    out.push_back(i);
                }
            }
        };
    }

private:
    std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition;
};
}
#endif