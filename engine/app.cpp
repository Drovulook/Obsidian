#include "app.h"

namespace ODEngine {
    void App::run() {
        while(!m_ODWindow.shouldClose()) {
            glfwPollEvents(); 
        }
    }
}