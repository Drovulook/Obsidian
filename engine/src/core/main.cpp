#include "app.h"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

extern ODEngine::App* ODEngine::CreateApp();

int main() {
    auto app = ODEngine::CreateApp();
    try {
        app->run();
    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}