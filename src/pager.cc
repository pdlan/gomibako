#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include "pager.h"

using namespace gomibako;

Pager::Pager(int items_per_page) {
    this->items_per_page = items_per_page;
}

Filter Pager::get_filter(int page, int &pages) {
    return [this, page, &pages] (const TimeIDMap &ids, const IDMetadataMap &metadata)
        -> std::shared_ptr<const TimeIDMap> {
        if (page <= 0) {
            return nullptr;
        }
        size_t size = ids.size();
        pages = ceil(double(size) / this->items_per_page);
        if (page > pages) {
            return nullptr;
        }
        int begin = (page - 1) * this->items_per_page;
        int end = page == pages ? size : page * this->items_per_page;
        int out_size = end - begin;
        std::shared_ptr<TimeIDMap> out(new TimeIDMap);
        std::copy_n(ids.begin(), out_size, std::inserter(*out, out->end()));
        return out;
    };
}

void gomibako::get_pagination(const TimeIDMap &ids, const IDMetadataMap &metadata,
                              const std::string &id, int items_per_page, int &page, int &pages) {
    size_t size = ids.size();
    pages = ceil(double(size) / items_per_page);
    page = 0;
    /*
    for (size_t i = 0; i < ids.size(); ++i) {
        if (ids[i].second == id) {
            page = ceil((i + 1) / (double)items_per_page);
        }
    }
    */
    auto &&it = metadata.find(id);
    if (it == metadata.end()) {
        return;
    }
    auto &&itpair = ids.equal_range(it->second.timestamp);
    size_t n = 0;
    for (auto i = itpair.first; i != itpair.second; ++i, ++n) {
        if (i->second == id) {
            page = ceil((n + 1) / (double)items_per_page);
            return;
        }
    }
}
