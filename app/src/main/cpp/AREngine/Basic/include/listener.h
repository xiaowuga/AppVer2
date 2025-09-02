#include <thread>
#include <atomic>
#include "BasicData.h"

void listenForEvent(AppData &appData) {
    std::cout << "Press [q] to exit..." << std::endl;
    while (appData._continue) {
        if (std::cin.good()) {
            std::cout << "while" << std::endl;
            char input;
            std::cin.get(input);  // 直接读取输入字符
            if (input == 'q') {
                appData._continue = false;
                std::cout << "Exit signal received." << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // clear buffer
                continue;
            }
            if (input == 's') {
                appData.record = true;
                std::cout << "Start Record signal received." << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // clear buffer
                continue;
            }
        } else {
            std::cin.clear(); // 清理错误状态
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 减少 CPU 使用率
    }
}