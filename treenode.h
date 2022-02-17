#pragma once

#include<vector>
#include<string>
#include<iostream>

class TreeNode
{
public:
    std::vector<std::string> includes;
    size_t counter = 0;
    bool exists = false;
    bool inspected = false;

    TreeNode();

    friend std::ostream& operator<< (std::ostream &out, const TreeNode &treenode);
};

