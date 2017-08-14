#ifndef GOMIBAKO_INCLUDE_FILTER_H
#define GOMIBAKO_INCLUDE_FILTER_H
#include <functional>
#include <unordered_map>
#include <vector>
#include "article.h"

namespace gomibako {
template<class T>
class CachedFilter {
public:
    CachedFilter(std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition_) : condition(condition_) {}

    Filter get_filter(const T &arg) {
        return [this, &arg] (const TimeIDMap &ids, const IDMetadataMap &metadata) -> std::shared_ptr<const TimeIDMap> {
            auto &&it = this->cache.find(arg);
            if (it == this->cache.end()) {
                std::shared_ptr<TimeIDMap> new_ids(new TimeIDMap);
                for (auto &&i : ids) {
                    if (this->condition(arg, metadata.at(i.second))) {
                        new_ids->insert(i);
                    }
                }
                this->cache[arg] = new_ids;
                return new_ids;
            } else {
                return it->second;
            }
        };
    }

    void clear_cache() {
        this->cache.clear();
    }

    ~CachedFilter() {
        clear_cache();
    }
private:
    std::unordered_map<T, std::shared_ptr<TimeIDMap>> cache;
    std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition;
};

template<class T>
class DirectFilter {
public:
    DirectFilter(std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition_) : condition(condition_) {}

    Filter get_filter(const T &arg) {
        return [this, &arg] (const TimeIDMap &ids, const IDMetadataMap &metadata) -> std::shared_ptr<const TimeIDMap> {
            std::shared_ptr<TimeIDMap> out(new TimeIDMap);
            for (auto &&i : ids) {
                if (this->condition(arg, metadata.at(i.second))) {
                    out->insert(i);
                }
            }
            return out;
        };
    }

private:
    std::function<bool (const T &arg, const ArticleMetadata &metadata)> condition;
};

}
#endif
