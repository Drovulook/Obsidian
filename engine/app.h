#pragma once

#include "window.h"

namespace od_engine {
    class App {
        public:
            static constexpr int WIDTH = 1000;
            static constexpr int HEIGHT = 700;

            void run();

        private:
            Window m_window{WIDTH, HEIGHT, "Obsidian Engine"};
    };
}