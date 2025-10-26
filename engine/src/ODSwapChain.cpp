#include "ODSwapChain.h"
#include "Particle.h"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace ODEngine {

ODSwapChain::ODSwapChain(ODDevice &deviceRef, VkExtent2D extent)
    : device{deviceRef}, windowExtent{extent} {
  init();
}

ODSwapChain::ODSwapChain(ODDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<ODSwapChain> previous)
: device{deviceRef}, windowExtent{windowExtent}, oldSwapChain{previous} {
  init();

  oldSwapChain = nullptr;  
}

void ODSwapChain::init(){
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createMsaaColorResources();
  createFramebuffers();
  createSyncObjects();
}

ODSwapChain::~ODSwapChain() {
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(device.device(), imageView, nullptr);
  }
  swapChainImageViews.clear();

  if (swapChain != nullptr) {
    vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
    swapChain = nullptr;
  }

  for (int i = 0; i < depthImages.size(); i++) {
    vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
    vkDestroyImage(device.device(), depthImages[i], nullptr);
    vkFreeMemory(device.device(), depthImageMemorys[i], nullptr);
  }
    // for multisampling
    vkDestroyImageView(device.device(), msaaColorImageView, nullptr);
    vkDestroyImage(device.device(), msaaColorImage, nullptr);
    vkFreeMemory(device.device(), msaaColorImageMemory, nullptr);

  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(device.device(), renderPass, nullptr);

  // cleanup synchronization objects
  for (size_t i = 0; i < imageCount(); i++) {
    vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
    vkDestroySemaphore(device.device(), computeFinishedSemaphores_[i], nullptr);
  }
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyFence(device.device(), inFlightFences[i], nullptr);
    vkDestroyFence(device.device(), computeInFlightFences_[i], nullptr);
  }
}

VkResult ODSwapChain::acquireNextImage(uint32_t *imageIndex) {
  vkWaitForFences(
      device.device(),
      1,
      &inFlightFences[currentFrame],
      VK_TRUE,
      std::numeric_limits<uint64_t>::max());

  VkResult result = vkAcquireNextImageKHR(
      device.device(),
      swapChain,
      std::numeric_limits<uint64_t>::max(),
      imageAvailableSemaphores[currentFrame],  // Utilise currentFrame pour l'acquisition
      VK_NULL_HANDLE,
      imageIndex);

  return result;
}

VkResult ODSwapChain::submitCommandBuffers(
    const VkCommandBuffer *buffers, uint32_t *imageIndex) {
  if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
  }
  imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // IMPORTANT: Utilise currentFrame pour wait (correspond à acquire) mais imageIndex pour signal
  VkSemaphore waitSemaphores[] = {computeFinishedSemaphores_[currentFrame], imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 2;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[*imageIndex]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
  if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = imageIndex;

  auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

// VkResult ODSwapChain::submitComputeCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex)
// {
//     vkWaitForFences(device.device(), 1, &computeInFlightFences_[currentFrame], VK_TRUE, UINT64_MAX);

//         vkResetFences(device.device(), 1, &computeInFlightFences_[currentFrame]);

//         vkResetCommandBuffer(computeCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
//         recordComputeCommandBuffer(computeCommandBuffers[currentFrame]);
//         VkSubmitInfo submitInfo{};
//         submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//         submitInfo.commandBufferCount = 1;
//         submitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
//         submitInfo.signalSemaphoreCount = 1;
//         submitInfo.pSignalSemaphores = &computeFinishedSemaphores_[currentFrame];

//         if (vkQueueSubmit(computeQueue, 1, &submitInfo, computeInFlightFences_[currentFrame]) != VK_SUCCESS) {
//             throw std::runtime_error("failed to submit compute command buffer!");
//         };
// }

// void ODSwapChain::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, FrameInfo &frameInfo) {
//   VkCommandBufferBeginInfo beginInfo{};
//   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

//   if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
//       throw std::runtime_error("failed to begin recording compute command buffer!");
//   }
//   m_odPipeline->bind(commandBuffer);

//   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

//   vkCmdDispatch(commandBuffer, ODParticles::PARTICLE_COUNT / 256, 1, 1);

//   if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
//       throw std::runtime_error("failed to record compute command buffer!");
//   }
// }

void ODSwapChain::waitForImageToBeAvailable(uint32_t imageIndex) {
  // Attendre que cette image ne soit plus en cours d'utilisation
  if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    // Vérifier d'abord si la fence est déjà signalée (non-bloquant)
    VkResult result = vkGetFenceStatus(device.device(), imagesInFlight[imageIndex]);
    if (result == VK_NOT_READY) {
      // Seulement attendre si nécessaire
      vkWaitForFences(device.device(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
  }
}

void ODSwapChain::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = device.surface();

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsAndComputeFamily, indices.presentFamily};

  if (indices.graphicsAndComputeFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

  if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

VkImageView ODSwapChain::createImageView(ODDevice &app_device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, uint32_t mipLevels){
  VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(app_device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

void ODSwapChain::createImageViews() {
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    swapChainImageViews[i] = createImageView(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void ODSwapChain::createRenderPass() {
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = device.getMsaaSamples();
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = getSwapChainImageFormat();
  colorAttachment.samples = device.getMsaaSamples();
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachmentResolve{};
  colorAttachmentResolve.format = swapChainImageFormat;
  colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentResolveRef{};
  colorAttachmentResolveRef.attachment = 2;
  colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pResolveAttachments = &colorAttachmentResolveRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstSubpass = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void ODSwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<VkImageView, 3> attachments = {msaaColorImageView, depthImageViews[i], swapChainImageViews[i]};

    VkExtent2D swapChainExtent = getSwapChainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
            device.device(),
            &framebufferInfo,
            nullptr,
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void ODSwapChain::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();
  swapChainDepthFormat = depthFormat;
  VkExtent2D swapChainExtent = getSwapChainExtent();

  depthImages.resize(imageCount());
  depthImageMemorys.resize(imageCount());
  depthImageViews.resize(imageCount());

  for (int i = 0; i < depthImages.size(); i++) {
    createImage(device, swapChainExtent.width, swapChainExtent.height, 1, device.getMsaaSamples(), depthFormat, VK_IMAGE_TILING_OPTIMAL, 
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageMemorys[i]);
      depthImageViews[i] = createImageView(device, depthImages[i], depthFormat, 
        VK_IMAGE_ASPECT_DEPTH_BIT, 1);  // Ajout de l'assignation depthImageViews[i] = ...
  }
}

void ODSwapChain::createImage(ODDevice& app_device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, 
  VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) {
    VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; 
        imageInfo.samples = numSamples;

        if (vkCreateImage(app_device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(app_device.device(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = app_device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(app_device.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(app_device.device(), image, imageMemory, 0);
}

void ODSwapChain::createMsaaColorResources() {
      VkFormat colorFormat = swapChainImageFormat;

    createImage(device, swapChainExtent.width, swapChainExtent.height, 1, device.getMsaaSamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL, 
      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, msaaColorImage, msaaColorImageMemory);
    msaaColorImageView = createImageView(device, msaaColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1); // vérifier
}

void ODSwapChain::createSyncObjects() {
  // Un sémaphore par image du swap chain (recommandation Vulkan)
  imageAvailableSemaphores.resize(imageCount());
  renderFinishedSemaphores.resize(imageCount());
  computeFinishedSemaphores_.resize(imageCount());
  
  // Fences par frame en vol
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);
  computeInFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  // Créer les sémaphores pour chaque image
  for (size_t i = 0; i < imageCount(); i++) {
    if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=VK_SUCCESS ||
        vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics synchronization objects for image!");
    }
  }

  // Créer les fences pour les frames
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics synchronization objects for frame!");
    }
    if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &computeFinishedSemaphores_[i]) ||
      vkCreateFence(device.device(), &fenceInfo, nullptr, &computeInFlightFences_[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create compute synchronization objects for a frame!");
    }
  }
}

VkSurfaceFormatKHR ODSwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && // ou VK_FORMAT_B8G8R8A8_UNORM pour pas de correction GAMMA
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR ODSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }

  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      std::cout << "Present mode: Immediate" << std::endl;
      return availablePresentMode;
    }
  }

  std::cout << "Present mode: V-Sync" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ODSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkFormat ODSwapChain::findDepthFormat() {
      return device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

}  // namespace ODEngine