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
using EachFunction = std::function<void(const std::string& file_name, int deep, bool existance)>;

// Детектор комментов вида /* ... */
bool cut_comment_block(std::string& line, bool is_comment_block_open)
{
    std::string block_comment_start = "/*";
    std::string block_comment_end = "*/";

    if (!is_comment_block_open)
    {
        while (line.find(block_comment_start) != std::string::npos)
        {
            auto start_pos = line.find(block_comment_start);
            auto end_pos = line.find(block_comment_end);

            if (end_pos == std::string::npos)
            {
                is_comment_block_open = true;
                line.erase(start_pos);
            }
            else
                line.erase(start_pos, end_pos + 2);
        }
    }
    else
    {
        auto end_pos = line.find(block_comment_end);
        if (end_pos == std::string::npos)
            line.erase(line.begin(), line.end());
        else
        {
            line.erase(0, end_pos + 2);
            is_comment_block_open = cut_comment_block(line, false);
        }

    }
    return is_comment_block_open;

}

// Парсер файла для обнаружения инклудов
std::vector<fs::path> parse_file(const fs::path& path, const std::vector<fs::path>& headers_directories)
{
    std::vector<fs::path> includes;
    std::ifstream file(path.string());

    std::string line;
    std::regex include_regex (" *#include *(\"|<)(.*)(\"|>).*");
    std::smatch include_match;

    bool is_comment_block_open = false;

    while (std::getline(file, line))
    {

        is_comment_block_open = cut_comment_block(line, is_comment_block_open);

        if (std::regex_match(line, include_match, include_regex))
        {
            if(include_match.size() != 4)
                continue;

            if (include_match[1] == "<")
            {
                for (const auto& header_dir : headers_directories)
                {
                    const auto& include_path = header_dir.parent_path() / fs::path{ include_match[2] };
                    bool path_was_included = false;
                    for (auto const& entry : fs::directory_iterator(header_dir))
                    {
                        if (!entry.is_directory() && entry.path() == include_path)
                        {
                            includes.emplace_back(include_path);
                            path_was_included = true;
                            break;
                        }
                    }
                    if (path_was_included)
                        break;
                }
            }
            else
            {
                const auto& include_path = path.parent_path()/fs::path{ include_match[2] };
                includes.emplace_back(include_path);
            }
        }

    }
    return includes;
}

// Рекурсивное наполнение хеш таблицы вида (путь узла, данные узла)
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

// DFC
void dfs(FileMap& file_map, const std::string& root_file, EachFunction func, int deep)
{
    func(file_map[root_file].file_name, deep, file_map[root_file].exists);

    if (!file_map[root_file].inspected)
    {
        file_map[root_file].inspected = true;
        for(const auto& child : file_map[root_file].includes)
            dfs(file_map, child, func, deep + 1);
        file_map[root_file].inspected = false;
    } else {
        throw std::runtime_error("Cyclic dependence is found");
    }
}

// Фунция вывода дерева
void print_tree_from_the_root_node(FileMap& file_map, const std::string& root_node)
{
    // deep можно заменить на стек, чтобы при циклической зависимости понять где она находится
    auto printFunc = [](const std::string& file_name, int deep, bool existance){
        std::cout << std::string(deep * 2, ' ');
        std::cout << file_name;
        if (!existance)
            std::cout << " (!)";
        std::cout << std::endl;
    };
    dfs(file_map, root_node, printFunc, 0);
}

// Парсер коммандной строки
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


    // Создаём счётчик и обнуляем inspected для прохода вдальнейшем
    for (auto& [path, tree_node]: file_map) {
        tree_node.inspected = false;
        include_counter.emplace_back(path, tree_node.counter);
    }

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
        // Выводим элементы с частотой включений 0, т.к. это корневые узлы
        if (elem.second == 0)
            print_tree_from_the_root_node(file_map, elem.first);
    }

    std::cout << "\n";

    // Вывод счётчика
    for (const auto& elem: include_counter)
    {
        std::cout << elem.first.filename().string() << " - " << elem.second << std::endl;
    }
}
