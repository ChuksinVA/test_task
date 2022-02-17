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

// Подзадачи 3
// 1) Обработай циклическую зависимость
//    1.1) Топологическая сортировка (куда её впихивать)
// 2) Обработка аргументов коммандной строки (осталась с прошлого дня)
//    2.1) Судя по гуглу в бусте что-то было (Как его использовать?)
// 3) Вывод результатов (Как лучше делать вывод при проходе в глубину)
// 4) cmake
// 5) Документация
// 6) Учет комментов


#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>
#include <treenode.h>
#include <utility>
#include <fstream>
#include <regex>
#include <algorithm>
#include <functional>


namespace fs = std::filesystem;

using FileMap = std::unordered_map<std::string, TreeNode>;
using IncludeCounter = std::vector<std::pair<std::string, int>>;
using EachFunction = std::function<void(const std::string& file_name, int deep)>;



std::vector<std::string> parse_file(std::ifstream& file, const std::string& path)
{
    std::vector<std::string> result;
    std::string line;

    fs::path dir_path { path };
    dir_path.remove_filename();

    std::regex include_regex ("#include *(\"|<)(.*)(\"|>).*");
    std::smatch include_match;
    while (std::getline(file, line))
    {
        if (std::regex_search(line, include_match, include_regex))
        {
            if(include_match.size() != 4)
                continue;

            result.emplace_back(dir_path.string().append(include_match[2]));
        }

    }
    return result;
}

void inspect(FileMap& file_map, const std::string& path)
{
    auto& node = file_map[path];

    if (node.inspected)
        return;

    node.inspected = true;
    std::ifstream file(path);
    node.exists = file.is_open();

    if (node.exists)
    {
        node.includes = parse_file(file, path);
        for (const auto& child : node.includes)
        {
            inspect(file_map, child);
            file_map[child].counter++;
        }
    }
}


void dfs(FileMap& file_map, const std::string& root_file, EachFunction func, int deep)
{
    fs::path path{root_file};
    func(path.filename().string(), deep);
    for(const auto& child : file_map[root_file].includes)
        dfs(file_map, child, func, deep + 1);
}


void print_file_map_from_root(FileMap& file_map, const std::string& root_file)
{
    auto printFunc = [](const std::string& file_name, int deep){
        std::cout << std::string(deep * 2, ' ');
        std::cout << file_name << std::endl;
    };
    dfs(file_map, root_file, printFunc, 0);
}


int main(int argc, char *argv[])
{
//    if (argc != 2)
//    {
//        std::cout << "Usage: " << argv[0] << " <path>\n";
//        return 1;
//    }

    std::string test_dir {"/home/mteejay/data/practice_in_test_task/"};

    FileMap file_map{};
    IncludeCounter include_counter{};

    if (!fs::exists(test_dir))
    {
        std::cout << "Path " << test_dir << " does not exist.\n";
        return 1;
    }


    // Проход по директории
    for (auto const& file : fs::recursive_directory_iterator(test_dir))
    {
        if (!file.is_directory())
        {
            auto path = file.path();
            if (path.extension() == ".cpp" || path.extension() == ".hpp" || path.extension() == ".h")
                inspect(file_map, file.path().string());
        }
    }


    // Создаём счётчик
    for (const auto& elem: file_map)
        include_counter.emplace_back(elem.first, elem.second.counter);

    // Cортируем счётчик
    auto custom_pair_sort = [](const std::pair<std::string, int>& first_member, const std::pair<std::string, int>& second_member) -> bool {
        if (first_member.second == second_member.second)
            return first_member.first < second_member.first;
        else
            return first_member.second > second_member.second;
    };

    std::sort(include_counter.begin(), include_counter.end(), custom_pair_sort);

    // Вывод деревьев
    for (const auto& elem: include_counter)
    {
        if (elem.second == 0)
            print_file_map_from_root(file_map, elem.first);
    }

    std::cout << "\n";
    // Вывод счётчика
    for (const auto& elem: include_counter)
    {
        fs::path path {elem.first};
        std::cout << path.filename().string() << " - " << elem.second << std::endl;
    }





//    // Тест parse_file()

//    std::ifstream file(test_file);
//    std::vector<std::string> check_parse {};

//    if (file.is_open())
//    {
//        check_parse = parse_file(file, test_file);
//    }

//    for (const auto& elem : check_parse)
//    {
//        std::cout << elem << "\n";
//    }


//    // Тест inspect()

//    inspect(file_map, counter, test_file);

//    for (auto& elem : file_map) {
//        fs::path path {elem.first};
//        std::cout << path.filename() << ": include counter - " << elem.second.counter <<"\n";
//        std::cout << elem.second << "\n";
//    }



//    // Тест path_cast()

//    std::string my_include {"include.h"};

//    auto modified_path = path_cast(my_include, test_file);

//    std::cout << test_file << "\n";
//    std::cout << modified_path << std::endl;


}
