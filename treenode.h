#pragma once

#include<vector>
#include<string>

class TreeNode
{
public:
    std::vector<std::string> includes;
    size_t counter = 0;
    bool exists = false;
    bool inspected = false;

    TreeNode();
};

