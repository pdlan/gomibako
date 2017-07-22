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