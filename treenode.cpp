#include "treenode.h"


std::ostream& operator<< (std::ostream &out, const TreeNode &treenode)
{
    std::cout << "[";
    for (auto& child : treenode.includes)
    {
        std::cout << child << " ";
    }
    std::cout << "]";
}
