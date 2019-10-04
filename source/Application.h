#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>    // NB. don't include windows.h (or fmt) after glfw

#include <vector>
#include <optional>

//----------------------------------------------------------------------------------------
struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

//----------------------------------------------------------------------------------------
struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities = {};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
class Application
{
  GLFWwindow* m_window              = nullptr;
  VkInstance m_instance             = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface            = VK_NULL_HANDLE;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device                 = VK_NULL_HANDLE;
  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;
  VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
  std::vector<VkImage> m_swapChainImages;
  std::vector<VkImageView> m_swapChainImageViews;
  VkFormat m_swapChainImageFormat;
  VkExtent2D m_swapChainExtent;
  VkRenderPass m_renderPass;
  VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
  VkPipeline m_graphicsPipeline;
  std::vector<VkFramebuffer> m_swapChainFramebuffers;
  VkCommandPool m_commandPool;
  std::vector<VkCommandBuffer> m_commandBuffers;
  std::vector<VkSemaphore> m_imageAvailableSemaphores;
  std::vector<VkSemaphore> m_renderFinishedSemaphores;
  std::vector<VkFence> m_inFlightFences;
  size_t m_currentFrame = 0;

  bool m_framebufferResized = false;

private:
  void setupDebugMessenger();

  void createInstance();
  bool checkValidationLayerSupport();
  std::vector<const char*> getRequiredExtensions();

  QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);
  void createSurface();
  void pickPhysicalDevice();
  int rateDeviceSuitability(const VkPhysicalDevice& device);
  bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);

  void createLogicalDevice();

  void createSwapChain();
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR
  chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR
  chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

  void createImageViews();
  void createRenderPass();

  VkShaderModule createShaderModule(const std::vector<char>& code);
  void createGraphicsPipeline();
  void createFramebuffers();
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();

  void drawFrame();

  void recreateSwapChain();
  void cleanupSwapChain();

  void cleanup();

public:
  ~Application() { cleanup(); }

  void init();
  void run();

  void setFramebufferResized() { m_framebufferResized = true; }
};

//----------------------------------------------------------------------------------------
