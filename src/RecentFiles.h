#pragma once

#include <string>
#include <vector>

class RecentFiles
{
public:
    std::string lastOpenLocation = "";
    std::vector<std::string> recentFiles;

    void push(std::string &fileLocation);
    void load(std::string &inString);
    void save(std::string &outString);

    static constexpr int MAX_RECENT_FILES = 10;
};
