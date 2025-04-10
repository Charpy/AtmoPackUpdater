#pragma once

#include <switch.h>

#include <borealis.hpp>
#include <json.hpp>
#include <regex>
#include <set>

#include "constants.hpp"

namespace util {
    void downloadArchive(const std::string& url, contentType type);
    void downloadArchive(const std::string& url, contentType type, long& status_code);
    void extractArchive(contentType type);
    std::string getContentsPath();
    std::string getErrorMessage(long status_code);
    bool isErista();
    bool cp(char *filein, char *fileout);
    std::string getPackVersion();
    bool set90dns();
    bool deleteTheme();
    int showDialogBoxBlocking(const std::string& text, const std::string& opt1, const std::string& opt2);
    int showDialogBoxBlocking(const std::string& text, const std::string& opt);
    std::string getAppPath();
    bool isExfat();
}