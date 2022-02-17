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

// Подзадача 5
// 1) Свой метод get_line()
// 2) Ошибка с раскруткой при цикле


#include <treenode.h>
#include <unordered_map>
#include <utility>
#include <fstream>
#include <regex>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <vector>



namespace fs = std::filesystem;

using FileMap = std::unordered_map<std::string, TreeNode>;
using NodeCount = std::pair<fs::path, int>;
using IncludeCounter = std::vector<NodeCount>;
using EachFunction = std::function<void(const std::string& file_name, int deep)>;



fs::path parse_include()
{

}

/* */
std::vector<fs::path> parse_file(const fs::path& path, const std::vector<fs::path>& headers_directories)
{
    std::vector<fs::path> includes;
    std::ifstream file(path.string());

    std::string line;
    std::regex include_regex (" *#include *(\"|<)(.*)(\"|>).*");
    std::smatch include_match;

    //std::regex open_comment_regex ("*(\/\*) *");
    //std::regex close_comment_regex (" *(\*\/) *");

    while (std::getline(file, line))
    {        

        if (std::regex_match(line, include_match, include_regex))
        {
            if(include_match.size() != 4)
                continue;

//            if (include_match[1] == "<")
//            {
//                for (const auto& header_dir : headers_directories)
//                {
//                    const auto& include_path = header_dir.parent_path() / fs::path{ include_match[2] };
//                    for (auto const& entry : fs::directory_iterator(header_dir))
//                        if (!entry.is_directory()) {}
//                }
//            }
//            else
//            {
                const auto& include_path = path.parent_path()/fs::path{ include_match[2] };
                includes.emplace_back(include_path);
//            }
        }

    }
    return includes;
}

void inspect(FileMap& file_map, const fs::path& path, const std::vector<fs::path>& headers_directories)
{
    auto& node = file_map[path.string()];

    if (node.inspected)
        return;

    node.inspected = true;
    node.file_name = path.filename();
    node.exists = fs::exists(path);

    if (node.exists)
    {
        node.includes = parse_file(path, headers_directories);
        for (const auto& child : node.includes)
        {
            inspect(file_map, child, headers_directories);
            file_map[child].counter++;
        }
    }

}


void dfs(FileMap& file_map, const std::string& root_file, EachFunction func, int deep)
{
    func(file_map[root_file].file_name, deep);

    if (!file_map[root_file].inspected)
    {
        file_map[root_file].inspected = true;
        for(const auto& child : file_map[root_file].includes)
            dfs(file_map, child, func, deep + 1);
    } else {

        throw std::runtime_error("");
    }
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

void parse_command_line_arguments(int argc, char* argv[], fs::path& directory, std::vector<fs::path>& headers_directories)
{
    if (argc < 2)
        throw std::runtime_error("Enter the parameters");

    if (!fs::exists(argv[1]))
        throw std::runtime_error("Entered directory does not exist");

    directory = argv[1];

    const std::vector<std::string_view> args(argv + 2, argv + argc);

    for (size_t i = 0; i < args.size(); i = i + 2)
    {
        if (args[i] == "-I" && (i + 1) < args.size())
        {
            if(!fs::exists(args[i + 1]))
                throw std::runtime_error("Entered include directory does not exist");

            headers_directories.emplace_back(args[i + 1]);
        }
        else
        {
            throw std::runtime_error("To define include directory path(s), enter a command:\n\tanalyzer <directory path> -I <first include directory path> -I <second include directory path> ...");
        }
    }
}

int main(int argc, char *argv[])
{
    fs::path dir;
    std::vector<fs::path> headers_directories;

    parse_command_line_arguments(argc, argv, dir, headers_directories);

    FileMap file_map{};
    IncludeCounter include_counter{};


    // Проход по директории
    for (auto const& entry : fs::recursive_directory_iterator(dir))
    {
        if (!entry.is_directory())
        {
            auto path {entry.path()};
            if (path.extension() == ".cpp" || path.extension() == ".hpp" || path.extension() == ".h")
                inspect(file_map, path, headers_directories);
        }
    }


    // Создаём счётчик
    for (const auto& [path, tree_node]: file_map)
        include_counter.emplace_back(path, tree_node.counter);

    // Лямбда функция для сортировки счётчика инклудов
    auto custom_pair_sort = [](const NodeCount& left_elem, const NodeCount& right_elem) -> bool {
        if (left_elem.second == right_elem.second)
            return left_elem.first < right_elem.first;
        else
            return left_elem.second > right_elem.second;
    };

    // Сортируем счётчик инклудов
    std::sort(include_counter.begin(), include_counter.end(), custom_pair_sort);

    // Вывод деревьев
    for (const auto& elem: include_counter)
    {
        for (auto& [path, tree_node]: file_map)
            tree_node.inspected = false;
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
