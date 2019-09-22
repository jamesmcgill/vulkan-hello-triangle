#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // NB. don't include windows.h (or fmt) after glfw

#include <vector>
#include <optional>

//------------------------------------------------------------------------------
struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

//------------------------------------------------------------------------------
class Application
{
  GLFWwindow* m_window  = nullptr;
  VkInstance m_instance = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface;
  VkPhysicalDevice m_physicalDevice;
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;

  void setupDebugMessenger();

  void createInstance();
  bool checkValidationLayerSupport();
  std::vector<const char*> getRequiredExtensions();

  QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);
  void createSurface();
  void pickPhysicalDevice();
  int rateDeviceSuitability(const VkPhysicalDevice& device);
  void createLogicalDevice();

  void cleanup();

public:
  ~Application() { cleanup(); }

  void init();
  void run();
};

//------------------------------------------------------------------------------
