#include "file.hpp"
#include <iostream>

int main() {
    try {
        // 写入文件
        epiphany::os::File file("test.txt", epiphany::os::WRITE);
        const char* data = "Hello, File!";
        file.write(data, strlen(data));

        // 读取文件
        epiphany::os::File inFile("test.txt", epiphany::os::READ);
        char buffer[256];
        size_t bytesRead = inFile.read(buffer, sizeof(buffer));
        buffer[bytesRead] = '\0';
        std::cout << "Read content: " << buffer << std::endl;

    } catch (const epiphany::os::FileException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}