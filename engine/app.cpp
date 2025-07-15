#include "app.h"

namespace od_engine {
    void App::run() {
        while(!m_window.shouldClose()) {
            glfwPollEvents(); 
        }
    }
}