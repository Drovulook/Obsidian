#pragma once

#include "ODDevice.h"
#include "../Common/FrameInfo.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace ODEngine {

template <typename HandleT>
static inline unsigned long long vk_handle_to_ull(HandleT h) {
#if (VK_USE_64_BIT_PTR_DEFINES == 1)
  // Représentation pointeur -> convertir via uintptr_t
  return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(h));
#else
  // Représentation entière -> conversion directe
  return static_cast<unsigned long long>(h);
#endif
}

class ODSwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  ODSwapChain(ODDevice &deviceRef, VkExtent2D windowExtent);
  ODSwapChain(ODDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<ODSwapChain> previous);

  ~ODSwapChain();

  ODSwapChain(const ODSwapChain &) = delete;
  ODSwapChain &operator=(const ODSwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
  VkRenderPass getRenderPass() { return renderPass; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  std::vector<VkImageView>& getImagesView() { return swapChainImageViews; }
  size_t imageCount() { return swapChainImages.size(); }
  std::vector<VkImage> getImages() { return swapChainImages; }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }
  std::vector<VkSemaphore> computeFinishedSemaphores() { return computeFinishedSemaphores_; }
  std::vector<VkFence> computeInFlightFences() { return computeInFlightFences_; }
  
  VkResult submitCommandBuffersWithoutPresent(const VkCommandBuffer *buffers, uint32_t *imageIndex, uint32_t frameIndex);
  void presentFrameWithSemaphore(uint32_t *imageIndex, VkSemaphore waitSemaphore);
  VkSemaphore getRenderFinishedSemaphore(uint32_t imageIndex) { return renderFinishedSemaphores[imageIndex]; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex, uint32_t frameIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex, uint32_t frameIndex);
  // VkResult submitComputeCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
  // void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, FrameInfo& frameInfo);
  void waitForImageToBeAvailable(uint32_t imageIndex);

  bool compareSwapFormats(const ODSwapChain &swapChain) const {
    return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
           swapChain.swapChainImageFormat == swapChainImageFormat;
  }

  static VkImageView createImageView(ODDevice &app_device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, uint32_t mipLevels = 1);
  static void createImage(ODDevice& app_device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
      VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

 private:
 void init();
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createMsaaColorResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat;
  VkFormat swapChainDepthFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  ODDevice &device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<ODSwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkSemaphore> computeFinishedSemaphores_;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  std::vector<VkFence> computeInFlightFences_;
  size_t currentFrame = 0;

  // for multisampling
  VkImage msaaColorImage;
  VkDeviceMemory msaaColorImageMemory;
  VkImageView msaaColorImageView;
};

}  // namespace ODEngine