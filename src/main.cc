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
    }/*
    gomibako.get_article_manager()->add_article("test", "test", time(nullptr), {"test", "test2"});
    gomibako.get_article_manager()->add_article("test", "test", time(nullptr), {"test2", "test3"});
    */
    gomibako.start();
    return 0;
}