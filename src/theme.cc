#include <string>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include "gomibako.h"
#include "theme.h"

using namespace gomibako;

Theme::Theme(const std::string &path) {
    this->path = path;
    this->handle = nullptr;
}

bool Theme::load() {
#ifdef _WIN32
    void *_handle = LoadLibrary(this->path.c_str());
#else
    void *_handle = dlopen(this->path.c_str(), RTLD_LAZY);
#endif
    if (!_handle) {
        return false;
    }
    this->handle = _handle;
    typedef bool (*InitTheme) (ThemeConfiguration &);
    typedef void (*RenderArticle)
        (std::ostringstream &, const ArticleMetadata &, const std::string &,
         const SiteInformation &, std::shared_ptr<URLMaker>);
    typedef void (*RenderPage)
        (std::ostringstream &, const std::vector<ArticleMetadata> &,
         const std::vector<std::ostringstream> &, int, int,
         const SiteInformation &, std::shared_ptr<URLMaker>);
    typedef void (*RenderTag)
        (std::ostringstream &, const std::vector<ArticleMetadata> &,
         const std::vector<std::ostringstream> &, const std::string &, int, int,
         const SiteInformation &, std::shared_ptr<URLMaker>);
    typedef void (*RenderArchives)
        (std::ostringstream &, const std::vector<ArticleMetadata> &,
         int, int, const SiteInformation &, std::shared_ptr<URLMaker>);
    typedef void (*RenderCustomPage)
        (std::ostringstream &, const CustomPage &, const SiteInformation &, std::shared_ptr<URLMaker>);
    typedef void (*RenderTags)
        (std::ostringstream &, const std::map<std::string, int> &tags,
         const SiteInformation &, std::shared_ptr<URLMaker>);
    typedef void (*RenderError)
        (std::ostringstream &, int code, const SiteInformation &, std::shared_ptr<URLMaker>);
#ifdef _WIN32
#define DLSYM GetProcAddress
#else
#define DLSYM dlsym
#endif
    InitTheme init_theme = (InitTheme)DLSYM(this->handle, "init");
    RenderArticle render_article = (RenderArticle)DLSYM(this->handle, "render_article");
    RenderPage render_page = (RenderPage)DLSYM(this->handle, "render_page");
    RenderTag render_tag = (RenderTag)DLSYM(this->handle, "render_tag");
    RenderArchives render_archives = (RenderArchives)DLSYM(this->handle, "render_archives");
    RenderCustomPage render_custom_page = (RenderCustomPage)DLSYM(this->handle, "render_custom_page");
    RenderTags render_tags = (RenderTags)DLSYM(this->handle, "render_tags");
    RenderError render_error = (RenderError)DLSYM(this->handle, "render_error");
#undef DLSYM
    if (!init_theme ||
        !render_article ||
        !render_page ||
        !render_tag ||
        !render_archives ||
        !render_custom_page ||
        !render_tags ||
        !render_error) {
        return false;
    }
    this->render_article = render_article;
    this->render_page = render_page;
    this->render_tag = render_tag;
    this->render_archives = render_archives;
    this->render_custom_page = render_custom_page;
    this->render_tags = render_tags;
    this->render_error = render_error;
    return init_theme(this->config);
}

const ThemeConfiguration &Theme::get_configuration() {
    return this->config;
}

Theme::~Theme() {
    if (this->handle) {
#ifdef _WIN32
        FreeLibrary(this->handle);
#else
        dlclose(this->handle);
#endif
    }
}