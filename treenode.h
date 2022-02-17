#pragma once

#include<vector>
#include<string>
#include<iostream>

class TreeNode
{
public:
    std::vector<std::string> includes;
    std::string file_name;
    size_t counter = 0;
    bool exists = false;
    bool inspected = false;

    TreeNode() = default;

    friend std::ostream& operator<< (std::ostream &out, const TreeNode &treenode);
};

