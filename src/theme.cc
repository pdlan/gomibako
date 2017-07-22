#include <string>
#include <iostream>
#include <dlfcn.h>
#include "gomibako.h"
#include "theme.h"

using namespace gomibako;

Theme::Theme(const std::string &path) {
    this->path = path;
    this->handle = nullptr;
}

bool Theme::load() {
    void *_handle = dlopen(this->path.c_str(), RTLD_LAZY);
    if (!_handle) {
        return false;
    }
    this->handle = _handle;
    typedef bool (*InitTheme) (ThemeConfiguration &);
    typedef void (*RenderArticle)
        (std::ostringstream &, const ArticleMetadata &, const std::string &,
         const SiteInformation &, std::shared_ptr<URLMaker>);
    InitTheme init_theme = (InitTheme)dlsym(this->handle, "init");
    if (!init_theme) {
        return false;
    }
    RenderArticle render_article = (RenderArticle)dlsym(this->handle, "render_article");
    if (!render_article) {
        return false;
    }
    this->render_article = render_article;
    return init_theme(this->config);
}

const ThemeConfiguration &Theme::get_configuration() {
    return this->config;
}

Theme::~Theme() {
    if (this->handle) {
        dlclose(this->handle);
    }
}