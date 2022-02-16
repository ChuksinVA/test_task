// Подзадачи 1
// 1) Надо пройтись по папке и найти все подходящие файлы (.hpp , .cpp)
// 2) Надо пройтисть по этим файлам и найти в них все #include
//    2.1) Что делать со включениями стандартной библиотеки
//    2.2) Что делать с комментами типа \* ... *\ и \\ (Есть ли разница? Как детектить? Регулярки?)
//    2.3) Левые включения (!), что делать с ними? (Мб какой-то идентификатор)
// 3) Какой будет алгоритм (надо собрать граф включений)
//    3.1) Пройтись по всему и на основе этого строить граф?
//    3.2) Проходиться рекурсивно?
// 4) Сборка и отправка (темный лес, гугли или отправляй в сыром виде)



// Подзадачи 2
// 1) DFS (от анордеред мap) на вывод результата
//    1.1) Топологическая сортировка (найти циклы)
// 2) Обработка аргументов коммандной строки
//    2.1) Первый вызов inspect (проходим по первому аргументу)
//    2.2) Пока что не учитываю <скобочки> (аргументы после корневой папки)
// 3) Получение полного пути из инклуда
// 4) Сортировка счетчика
// 5) Оформи TreeNode
// 6) Вывод результата (мб метод в treenode?)
// 7) сMake (посмотри документацию)


#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>
#include <treenode.h>
#include <utility>
#include <fstream>
#include <regex>


namespace fs = std::filesystem;

using FileMap = std::unordered_map<std::string, TreeNode>;
using IncludeCounter = std::vector<std::pair<std::string, int>>;


std::vector<std::string> parse_file(std::ifstream& file)
{
    std::vector<std::string> result;
    std::string line;
    std::regex include_regex ("#include *(\"|<)(.*)(\"|>).*");
    std::smatch include_match;
    while (std::getline(file, line))
    {
        if (std::regex_search(line, include_match, include_regex))
        {
            if(include_match.size() != 3)
            {
                continue;
            }
            result.emplace_back(include_match[1]);
        }

    }
    return result;
}

void inspect(FileMap& file_map, IncludeCounter& include_counter, const std::string& path)
{
    auto& node = file_map[path];

    if (node.inspected)
    {
        return;
    }

    node.inspected = true;
    std::ifstream file(path);
    node.exists = file.is_open();

    if (node.exists)
    {
        node.includes = parse_file(file);
        for (const auto& child : node.includes)
        {
            inspect(file_map, include_counter, child);
            file_map[child].counter++;
        }
    }
}


int main(int argc, char *argv[])
{
    //std::string dir

/*
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <path>\n";
        return 1;
    }

    const fs::path dir {argv[1]};

    if (!exists(dir))
    {
        std::cout << "Path " << dir << " does not exist.\n";
        return 1;
    }

    for (const fs::directory_entry &e : fs::directory_iterator{dir})
    {
        if (e.is_directory())
        {

        }
        if (e.path().extension() == ".cpp" || e.path().extension() == ".h" || e.path().extension() == ".hpp" )
        {
            std::cout << e.path().filename() << std::endl;
        }
    }
*/
}
