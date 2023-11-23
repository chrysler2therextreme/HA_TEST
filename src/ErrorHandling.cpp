#include "ErrorHandling.h"

const unsigned long max_size = std::numeric_limits<std::streamsize>::max();

namespace ErrorHandling {
    int getIntInput(const std::string& prompt) {
        int value;

        std::cout << prompt;
        std::cin >> value;

        while (std::cin.fail()) {
            std::cout << "잘못입력했습니다. 다시 정수를 입력하세요:";
            std::cin.clear();
            std::cin.ignore(max_size, '\n');

            std::cin >> value;
            std::cin.ignore(max_size, '\n');
        }

        return value;
    }
}