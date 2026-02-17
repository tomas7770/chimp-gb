#include "RecentFiles.h"

#include <sstream>
#include <algorithm>

void RecentFiles::push(std::string &fileLocation)
{
    auto existingEntry = std::find(recentFiles.begin(), recentFiles.end(), fileLocation);
    if (existingEntry != recentFiles.end())
    {
        recentFiles.erase(existingEntry);
    }

    recentFiles.insert(recentFiles.begin(), fileLocation);

    if (recentFiles.size() > MAX_RECENT_FILES)
    {
        recentFiles.pop_back();
    }
}

void RecentFiles::load(std::string &inString)
{
    std::istringstream inputStream(inString);
    std::string line;
    std::getline(inputStream, line);
    lastOpenLocation = line;

    std::vector<std::string> lines;
    while (std::getline(inputStream, line))
    {
        lines.push_back(line);
    }
    for (int i = lines.size() - 1; i >= 0; i--)
    {
        push(lines.at(i));;
    }
}

void RecentFiles::save(std::string &outString)
{
    outString = lastOpenLocation;
    for (std::string &file : recentFiles)
    {
        outString += "\n" + file;
    }
}
