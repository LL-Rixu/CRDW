#include <SKSE/SKSE.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

std::string trim(const std::string& str)
{
    auto start = std::find_if_not(str.begin(), str.end(), [](int c) { return std::isspace(c); });
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](int c) { return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : "";
}

std::tuple<bool, std::string, size_t> parseini(void)
{
    const std::filesystem::path filename = "Data/SKSE/Plugins/CRDW.ini";

    bool showpopup;
    std::string mode;
    size_t iobuffer = 65568;
    std::ifstream file(filename);
    
    if (!file.is_open())
    {
        return { true, "LOAD", iobuffer };
    }

    std::string line;
    std::string currentSection = "";

    while (std::getline(file, line))
    {
        auto commentPos = line.find(';');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        line = trim(line);

        if (line.empty()) continue;

        if (line.front() == '[' && line.back() == ']')
        {
            currentSection = trim(line.substr(1, line.length() - 2));
            continue;
        }

        auto eqPos = line.find('=');

        if (eqPos != std::string::npos)
        {
            std::string key = trim(line.substr(0, eqPos));
            std::string val = trim(line.substr(eqPos + 1));

            if (currentSection == "General")
            {
                if(key == "ShowPopUp")
                {
                    std::string lowerVal = val;
                    std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), ::tolower);
                    showpopup = (lowerVal == "true" || lowerVal == "1");
                }
                else if(key == "iobuffer")
                {
                    iobuffer = std::stoi(val);
                }
            }
            else if (currentSection == "Mode" && key == "Mode")
            {
                mode = val;
            }
        }
    }

    return { showpopup, mode, iobuffer };
}