#pragma once

#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct TreeNode
{
    std::vector<fs::path> includes;
    std::string file_name;
    size_t counter = 0;
    bool exists = false;
    bool inspected = false;

};

