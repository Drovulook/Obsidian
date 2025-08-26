#pragma once

#include <ObsidianEngine.h>

namespace ODEngine {
    class SandboxApp : public App {
    public:
        SandboxApp();
        ~SandboxApp();

        void run() override;

    protected:
        void loadGameObjects() override;

    private:

        void create3DObjFromFile(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale,  std::shared_ptr<ODModel> model);
        
    };
}
