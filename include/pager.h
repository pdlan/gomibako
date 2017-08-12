#ifndef GOMIBAKO_INCLUDE_PAGER_H
#define GOMIBAKO_INCLUDE_PAGER_H
#include "article.h"

namespace gomibako {
class Pager {
public:
    Pager(int items_per_page);
    Filter get_filter(int page, int &pages);
private:
    int items_per_page;
};

void get_pagination(const TimeIDMap &ids, const IDMetadataMap &metadata,
                    const std::string &id, int items_per_page, int &page, int &pages);
}
#endif