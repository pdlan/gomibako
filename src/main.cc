#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <ctime>
#include "gomibako.h"
#include "pager.h"

using namespace gomibako;

int main(int argc, const char **argv) {
    Gomibako & gomibako = Gomibako::get_instance();
    if (!gomibako.initialize("config.yaml")) {
        return -1;
    }
    //gomibako.get_article_manager()->add_article("测试", "测试内容", time(nullptr), {"中文标签"});
    gomibako.start();
    return 0;
}