#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_VULKAN_IMPLEMENTATION

#include "nuklear.h"
#include "nuklear_glfw_vulkan.h"

#include "UIManager.h"
#include <iostream>
#include <stdexcept>

namespace ODEngine {
    UIManager::UIManager() {
        std::cout << "UIManager initialized." << std::endl;
    }

    UIManager::~UIManager() {
        if (m_initialized) {
            nk_glfw3_shutdown();
            m_initialized = false;
            m_context = nullptr;
        }
        std::cout << "UIManager destroyed." << std::endl;
    }

    void UIManager::init(GLFWwindow *win, VkDevice logical_device, VkPhysicalDevice physical_device, 
        uint32_t graphics_queue_family_index, VkImageView *image_views, uint32_t image_views_len, 
        VkFormat color_format, VkQueue graphics_queue) {

        m_window = win;
        m_logical_device = logical_device;
        m_physical_device = physical_device;
        m_graphics_queue_family_index = graphics_queue_family_index;
        m_color_format = color_format;
        m_graphics_queue = graphics_queue;

        m_context = nk_glfw3_init(win, logical_device, physical_device, graphics_queue_family_index, 
            image_views, image_views_len, color_format, NK_GLFW3_INSTALL_CALLBACKS, 512 * 1024, 128 * 1024);
            
        if (m_context) {
            m_initialized = true;
            
            // Configuration des polices
            struct nk_font_atlas *atlas;
            nk_glfw3_font_stash_begin(&atlas);
            // ajpouter des polices personnalisées ...
            nk_glfw3_font_stash_end(graphics_queue);
            std::cout << "Nuklear initialized successfully." << std::endl;
        } else {
            std::cerr << "Failed to initialize Nuklear." << std::endl;
        }
        
   }

   void UIManager::render() {
        if (!m_initialized || !m_context) return;

        // Exemple d'interface simple
        if (nk_begin(m_context, "Demo", nk_rect(m_demoWindowX, m_demoWindowY, m_demoWindowW, m_demoWindowH),
                    NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                    NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {

            nk_layout_row_static(m_context, 30, 80, 1);
            if (nk_button_label(m_context, "button")) {
                std::cout << "Button pressed!" << std::endl;
            }
        }

        // Sauvegarder la position/taille actuelle (avant nk_end, pendant que la fenêtre est active)
        struct nk_rect bounds = nk_window_get_bounds(m_context);
        m_demoWindowX = bounds.x;
        m_demoWindowY = bounds.y;
        m_demoWindowW = bounds.w;
        m_demoWindowH = bounds.h;

        nk_end(m_context);
   }

   void UIManager::handleInput() {

   }

   void UIManager::newFrame() {
    if (m_initialized) {
            nk_glfw3_new_frame();
        }
   }

   VkSemaphore UIManager::renderUI(VkQueue graphicsQueue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
       if (!m_initialized) return VK_NULL_HANDLE;
        return nk_glfw3_render(graphicsQueue, imageIndex, waitSemaphore, NK_ANTI_ALIASING_ON);
   }
   
   void UIManager::updateResources(VkImageView *image_views, uint32_t image_views_len, uint32_t framebuffer_width, uint32_t framebuffer_height) {
    if (!m_initialized) {
           std::cerr << "UIManager not initialized; cannot update resources." << std::endl;
           return;
       }
       
       std::cout << "Updating UIManager resources to " << framebuffer_width << "x" << framebuffer_height << std::endl;
       
       // Attendre que le device soit idle avant de détruire les ressources
       vkDeviceWaitIdle(m_logical_device);
       
       // Détruire l'ancienne instance Nuklear
       nk_glfw3_shutdown();
       m_initialized = false;
       m_context = nullptr;
       
       // Recréer avec les nouvelles ressources (sans réinstaller les callbacks GLFW)
       m_context = nk_glfw3_init(m_window, m_logical_device, m_physical_device,
                                m_graphics_queue_family_index, image_views, image_views_len,
                                m_color_format, NK_GLFW3_DEFAULT, 512 * 1024, 128 * 1024);
           
       if (m_context) {
           m_initialized = true;
           
           // Reconfigurer les polices
           struct nk_font_atlas *atlas;
           nk_glfw3_font_stash_begin(&atlas);
           nk_glfw3_font_stash_end(m_graphics_queue);
           
           std::cout << "UIManager resources updated successfully." << std::endl;
       } else {
           std::cerr << "Failed to update UIManager resources." << std::endl;
       }
   }

} // namespace ODEngine