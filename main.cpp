#include <iostream>
#include "Utils.h"

int main() {
    while (1) {
        std::cout << random::xorshf96() << std::endl;
    }
    return 0;
}
