#pragma once

#include <memory>

struct nk_context;
struct nk_font_atlas;

namespace ODEngine {
    class UIManager {
    public:
        UIManager();
        ~UIManager();

        void init(GLFWwindow *win, VkDevice logical_device, VkPhysicalDevice physical_device, 
            uint32_t graphics_queue_family_index, VkImageView *image_views, uint32_t image_views_len, 
            VkFormat color_format, VkQueue graphics_queue);
        void render();
        void handleInput();
        void newFrame();
        VkSemaphore renderUI(VkQueue graphicsQueue, uint32_t imageIndex, VkSemaphore waitSemaphore);

    private:
        nk_context *m_context;
        nk_font_atlas *m_fontAtlas;
        bool m_initialized;
    };
}