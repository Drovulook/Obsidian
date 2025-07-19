#include "SandboxApp.h"

#include <iostream>

namespace ODEngine {
    
    SandboxApp::SandboxApp() {
    }

    SandboxApp::~SandboxApp() {
    }

    void SandboxApp::run() {
        std::cout << "Running Sandbox Application..." << std::endl;
        App::run();
    }

    App* CreateApp() {
        return new SandboxApp();
    }
}