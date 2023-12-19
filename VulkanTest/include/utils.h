#include <fstream>
#include <vector>

namespace EngineUtils{
    static std::vector<char> readFile(const std::string &filename)
    {
        //从文件末尾开始读取，读入为字节流
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }


}