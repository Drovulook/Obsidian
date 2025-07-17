#include "app.h"

namespace ODEngine {
    void App::run() {
        while(!m_window.shouldClose()) {
            glfwPollEvents(); 
        }
    }
}