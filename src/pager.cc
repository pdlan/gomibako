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
    return [this, page, &pages] (const TimeIDVector &ids, const IDMetadataMap &metadata, TimeIDVector &out) {
        if (page <= 0) {
            return;
        }
        size_t size = ids.size();
        pages = ceil(double(size) / this->items_per_page);
        if (page > pages) {
            return;
        }
        int begin = (page - 1) * this->items_per_page;
        int end = page == pages ? size : page * this->items_per_page;
        int out_size = end - begin;
        out.resize(out_size);
        std::copy(ids.cbegin() + begin, ids.cbegin() + end, out.begin());
    };
}

void gomibako::get_pagination(const TimeIDVector &ids, const IDMetadataMap &metadata, TimeIDVector &out,
                    const std::string &id, int items_per_page, int &page, int &pages) {
    size_t size = ids.size();
    pages = ceil(double(size) / items_per_page);
    page = 0;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (ids[i].second == id) {
            page = ceil((i + 1) / (double)items_per_page);
        }
    }
}