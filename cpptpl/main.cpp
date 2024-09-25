#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

// 自定义INI文件解析器
class INIParser {
public:
    void parse(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::string line;
        std::string current_section;
        while (std::getline(file, line)) {
            // 去除空格和注释
            line = trim(line);
            if (line.empty() || line[0] == ';') {
                continue; // 跳过空行和注释
            }

            // 处理节
            if (line[0] == '[' && line[line.size() - 1] == ']') {
                current_section = line.substr(1, line.size() - 2);
                continue;
            }

            // 处理键值对
            std::size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));
                data[current_section][key] = value;
            }
        }
        file.close();
    }

    std::string get(const std::string& section, const std::string& key) {
        return data[section][key];
    }

private:
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data;
};

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    std::string name;
    int age;
    bool verbose;

    // 定义一个options_description对象，用于存储命令行参数
    po::options_description desc("allow options");
    // 添加命令行参数
    desc.add_options()
        // 添加一个help参数，用于显示帮助信息
        ("help", "show help message")
        // 添加一个name参数，用于存储用户名，并设置为必填项
        ("name,n", po::value<std::string>(&name)->required(), "your name")
        // 添加一个age参数，用于存储用户年龄，并设置默认值为-1
        ("age,a", po::value<int>(&age)->default_value(-1), "your age")
        // 添加一个verbose参数，用于控制是否显示详细信息，并设置默认值为false
        ("verbose,v", po::bool_switch(&verbose)->default_value(false), "whether show detailed information");
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "failed: " << e.what() << "\n";
        std::cerr << desc << "\n";
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    // 输出结果
    if (verbose) {
        std::cout << "received params:\n";
        std::cout << "name: " << name << "\n";
        std::cout << "age: " << age << "\n";
        std::cout << "detailed information: " << (verbose ? "yes" : "no") << "\n";
    }

    std::cout << "Hello, " << name << "!\n";
    if (age != -1) {
        std::cout << "your age is: " << age << "\n";
    }


    printf("1------------------>\n");
    // 读取INI文件
    INIParser parser;
    parser.parse("cpptpl.ini");

    // 获取配置
    std::string value = parser.get("Section1", "Key1");
    std::cout << "Section1.Key1: " << value << value.length() << std::endl;
    std::cout << "Section1.Key133: " << parser.get("Section1", "Key133") << std::endl;

    return 0;
}
