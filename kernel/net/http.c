#include "../include/types.h"

static const char demo_page_example[] =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head><title>Example.com</title></head>\n"
    "<body>\n"
    "<h1>Example Domain</h1>\n"
    "<p>This domain is for use in examples and documentation.</p>\n"
    "<p>More information...</p>\n"
    "</body>\n"
    "</html>\n";

static const char demo_page_google[] =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head><title>Google</title></head>\n"
    "<body>\n"
    "<h1>Google Search</h1>\n"
    "<p>Search the web with Google</p>\n"
    "<input type=\"text\" placeholder=\"Search...\">\n"
    "</body>\n"
    "</html>\n";

static const char demo_page_github[] =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head><title>GitHub</title></head>\n"
    "<body>\n"
    "<h1>GitHub</h1>\n"
    "<p>Build software better, together</p>\n"
    "<p>GitHub is where the world builds software.</p>\n"
    "</body>\n"
    "</html>\n";

typedef struct {
    const char *url;
    const char *content;
    const char *content_type;
} DemoContent;

static const DemoContent demo_pages[] = {
    {"http://example.com", demo_page_example, "text/html"},
    {"http://www.example.com", demo_page_example, "text/html"},
    {"http://google.com", demo_page_google, "text/html"},
    {"http://www.google.com", demo_page_google, "text/html"},
    {"http://github.com", demo_page_github, "text/html"},
    {"http://www.github.com", demo_page_github, "text/html"},
    {0, 0, 0}
};

const char *http_get_demo_content(const char *url) {
    u32 i = 0;
    while (demo_pages[i].url) {
        const char *demourl = demo_pages[i].url;
        const char *inputurl = url;

        u32 match = 1;
        while (*demourl && *inputurl && match) {
            if (*demourl != *inputurl) {
                match = 0;
            }
            ++demourl;
            ++inputurl;
        }

        if (match && !*demourl) {
            return demo_pages[i].content;
        }

        ++i;
    }

    return 0;
}

u32 http_measure_content_length(const char *html) {
    u32 len = 0;
    while (html && html[len]) {
        ++len;
    }
    return len;
}
