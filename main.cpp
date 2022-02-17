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
// 7) сmake (посмотри документацию)

// Подзадачи 3
// 1) Обработай циклическую зависимость
//    1.1) Топологическая сортировка (куда её впихивать)
// 2) Обработка аргументов коммандной строки (осталась с прошлого дня)
//    2.1) Судя по гуглу в бусте что-то было (Как его использовать?)
// 3) Вывод результатов (Как лучше делать вывод при проходе в глубину)
// 4) cmake
// 5) Документация
// 6) Учет комментов

// Подзадачи 4
// 1) Циклическая зависимость (Как бы её корректно выводить??)
// 2) Учет комментов
// 3) Треугольные скобочки
//    3.1) Отказаться от string??
// 4) cmake
// 6) Документация


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

std::vector<std::string> parse_file(std::ifstream& file, fs::path path)
{
    std::vector<std::string> result;
    std::string line;

    std::regex include_regex ("#include *(\"|<)(.*)(\"|>).*");
    std::smatch include_match;
    while (std::getline(file, line))
    {
        if (std::regex_search(line, include_match, include_regex))
        {
            if(include_match.size() != 4)
                continue;

            result.emplace_back(path.remove_filename().string().append(include_match[2]));
        }

    }
    return result;
}

void inspect(FileMap& file_map, const fs::path& path)
{
    auto& node = file_map[path.string()];

    if (node.inspected)
        return;

    node.inspected = true;
    node.file_name = path.filename();
    std::ifstream file(path.string());
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


// analyzer d:\mysources\ -I d:\mysources\includes -I d:\mylibrary

void parse_command_line_arguments(int argc, char* argv[], std::string& directory, std::vector<std::string>& include_directories)
{
    if (argc < 2)
        throw std::runtime_error("Enter the parameters");

    if (!fs::exists(argv[1]))
        throw std::runtime_error("Entered directory does not exist");

    directory = argv[1];

    const std::vector<std::string_view> args(argv + 2, argv + argc);

    for (size_t i = 0; i < args.size(); i = i + 2)
    {
        if (args[i] == "I" && (i + 1) < args.size())
        {
            if(!fs::exists(args[i + 1]))
                throw std::runtime_error("Entered include directory does not exist");

            include_directories.emplace_back(args[i + 1]);
        }
        else
        {
            throw std::runtime_error("To define include directory path(s), enter a command:\n\tanalyzer <directory path> I <first include directory path> I <second include directory path> ...");
        }
    }
}

int main(int argc, char *argv[])
{

    std::string dir;
    std::vector<std::string> include_directories;

    parse_command_line_arguments(argc, argv, dir, include_directories);

    FileMap file_map{};
    IncludeCounter include_counter{};


    // Проход по директории
    for (auto const& file : fs::recursive_directory_iterator(dir))
    {
        if (!file.is_directory())
        {
            auto path {file.path()};
            if (path.extension() == ".cpp" || path.extension() == ".hpp" || path.extension() == ".h")
                inspect(file_map, path);
        }
    }


    // Создаём счётчик
    for (auto& elem: file_map)
    {
        include_counter.emplace_back(elem.first, elem.second.counter);
    }
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
