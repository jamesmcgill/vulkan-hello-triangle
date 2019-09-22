// always include fmt before Application.h (includes windows.h and so does glfw)
#include <fmt/format.h>

#include "Application.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <map>
#include <set>
#include <algorithm>

//------------------------------------------------------------------------------
constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

const std::vector<const char*> DEVICE_EXTENSIONS
  = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<const char*> VALIDATION_LAYERS
  = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

static VkDebugUtilsMessengerEXT sg_debugMessenger;

//------------------------------------------------------------------------------
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData)
{
  UNREFERENCED_PARAMETER(messageType);
  UNREFERENCED_PARAMETER(pUserData);

  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
  {
    fmt::print("VALIDATION LAYER: {}\n", pCallbackData->pMessage);
  }
  return VK_FALSE;
}

//------------------------------------------------------------------------------
static VkResult
CreateDebugUtilsMessengerEXT(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
  const VkAllocationCallbacks* pAllocator,
  VkDebugUtilsMessengerEXT* pDebugMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
    instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

//------------------------------------------------------------------------------
static void
DestroyDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerEXT debugMessenger,
  const VkAllocationCallbacks* pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
    instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    func(instance, debugMessenger, pAllocator);
  }
}

//------------------------------------------------------------------------------
static void
populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
  createInfo       = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Application Implementation
//------------------------------------------------------------------------------
void
Application::setupDebugMessenger()
{
  assert(m_instance != VK_NULL_HANDLE);

  if (!ENABLE_VALIDATION_LAYERS)
  {
    return;
  }

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  if (
    CreateDebugUtilsMessengerEXT(
      m_instance, &createInfo, nullptr, &sg_debugMessenger)
    != VK_SUCCESS)
  {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

//------------------------------------------------------------------------------
void
Application::createInstance()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);    // not using OpenGL
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);      // not resizable
  m_window = glfwCreateWindow(
    WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan hello triangle", nullptr, nullptr);

  if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
  {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  auto reqExtensions = getRequiredExtensions();

  uint32_t vkExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
  std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
  vkEnumerateInstanceExtensionProperties(
    nullptr, &vkExtensionCount, vkExtensions.data());

  fmt::print("available extensions:\n");
  for (const auto& extension : vkExtensions)
  {
    fmt::print("\t {}\n", extension.extensionName);
  }
  fmt::print("required extensions:\n");
  fmt::print("\t{}\n", fmt::join(reqExtensions, ",\n\t"));

  VkApplicationInfo appInfo  = {};
  appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName   = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName        = "No Engine";
  appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion         = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo     = &appInfo;
  createInfo.enabledExtensionCount
    = static_cast<uint32_t>(reqExtensions.size());
  createInfo.ppEnabledExtensionNames = reqExtensions.data();

  // Must live in same scope as vkCreateInstance and createInfo
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (ENABLE_VALIDATION_LAYERS)
  {
    createInfo.enabledLayerCount
      = static_cast<uint32_t>(VALIDATION_LAYERS.size());
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  }

  if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
  {
    throw std::runtime_error("Couldn't create VkInstance!");
  }
}
//------------------------------------------------------------------------------
bool
Application::checkValidationLayerSupport()
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char* layerName : VALIDATION_LAYERS)
  {
    bool layerFound = false;
    for (const auto& layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }
    if (!layerFound)
    {
      fmt::print("Validation layer {} not found\n", layerName);
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------------------
std::vector<const char*>
Application::getRequiredExtensions()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions
    = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(
    glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (ENABLE_VALIDATION_LAYERS)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}

//------------------------------------------------------------------------------
QueueFamilyIndices
Application::findQueueFamilies(const VkPhysicalDevice& device)
{
  assert(device != VK_NULL_HANDLE);
  assert(m_surface != VK_NULL_HANDLE);

  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
    device, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies)
  {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport)
    {
      indices.presentFamily = i;
    }

    if (
      queueFamily.queueCount > 0
      && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      indices.graphicsFamily = i;
    }

    if (indices.isComplete())
    {
      break;
    }
    i++;
  }

  return indices;
}

//------------------------------------------------------------------------------
void
Application::createSurface()
{
  assert(m_instance != VK_NULL_HANDLE);
  assert(m_window != nullptr);

  if (
    glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface)
    != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create window surface!");
  }
}

//------------------------------------------------------------------------------
void
Application::pickPhysicalDevice()
{
  assert(m_instance != VK_NULL_HANDLE);

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

  if (deviceCount == 0)
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

  std::multimap<int, VkPhysicalDevice> devicesByScore;
  for (const auto& device : devices)
  {
    int score = rateDeviceSuitability(device);
    devicesByScore.insert(std::make_pair(score, device));
  }

  if (!devicesByScore.empty() && devicesByScore.rbegin()->first > 0)
  {
    m_physicalDevice = devicesByScore.rbegin()->second;
  }
  else
  {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

//------------------------------------------------------------------------------
int
Application::rateDeviceSuitability(const VkPhysicalDevice& device)
{
  assert(device != VK_NULL_HANDLE);

  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  // Hard requirements
  QueueFamilyIndices indices = findQueueFamilies(device);
  if (!indices.isComplete())
  {
    return 0;
  }
  if (!checkDeviceExtensionSupport(device))
  {
    return 0;
  }
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
  if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
  {
    return 0;
  }

  // Optional features weighted by value
  int score = 0;

  // Graphics and presentation using the same family is more performant
  if (indices.graphicsFamily == indices.presentFamily)
  {
    score += 100;
  }
  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
  {
    score += 1000;
  }
  score += deviceProperties.limits.maxImageDimension2D;

  return score;
}

//------------------------------------------------------------------------------
bool
Application::checkDeviceExtensionSupport(const VkPhysicalDevice& device)
{
  assert(device != VK_NULL_HANDLE);

  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(
    device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
    device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(
    DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
  for (const auto& extension : availableExtensions)
  {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

//------------------------------------------------------------------------------
void
Application::createLogicalDevice()
{
  assert(m_physicalDevice != VK_NULL_HANDLE);

  QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies
    = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount
    = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures  = &deviceFeatures;
  createInfo.enabledExtensionCount
    = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
  createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
  if (ENABLE_VALIDATION_LAYERS)
  {
    createInfo.enabledLayerCount
      = static_cast<uint32_t>(VALIDATION_LAYERS.size());
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  }

  if (
    vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device)
    != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(
    m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
  vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

//------------------------------------------------------------------------------
void
Application::createSwapChain()
{
  assert(m_physicalDevice != VK_NULL_HANDLE);
  assert(m_surface != VK_NULL_HANDLE);

  SwapChainSupportDetails swapChainSupport
    = querySwapChainSupport(m_physicalDevice);
  VkSurfaceFormatKHR surfaceFormat
    = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode
    = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = std::min(
    swapChainSupport.capabilities.minImageCount + 1,
    swapChainSupport.capabilities.maxImageCount);

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface          = m_surface;
  createInfo.minImageCount    = imageCount;
  createInfo.imageFormat      = surfaceFormat.format;
  createInfo.imageColorSpace  = surfaceFormat.colorSpace;
  createInfo.imageExtent      = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // QueueFamily sharing
  QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
  // NB. must remain in scope of createInfo
  uint32_t queueFamilyIndices[]
    = {indices.graphicsFamily.value(), indices.presentFamily.value()};
  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices   = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;          // Optional
    createInfo.pQueueFamilyIndices   = nullptr;    // Optional
  }

  createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode    = presentMode;
  createInfo.clipped        = VK_TRUE;
  createInfo.oldSwapchain   = VK_NULL_HANDLE;

  if (
    vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain)
    != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create swap chain!");
  }

  // Get Images: (find actual imageCount of created swap chain)
  vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
  m_swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(
    m_device, m_swapChain, &imageCount, m_swapChainImages.data());

  m_swapChainImageFormat = surfaceFormat.format;
  m_swapChainExtent      = extent;
}

//------------------------------------------------------------------------------
SwapChainSupportDetails
Application::querySwapChainSupport(VkPhysicalDevice device)
{
  assert(device != VK_NULL_HANDLE);
  assert(m_surface != VK_NULL_HANDLE);

  SwapChainSupportDetails details;

  // Surface Caps
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    device, m_surface, &details.capabilities);

  // Surface Formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
    device, m_surface, &formatCount, nullptr);
  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, m_surface, &formatCount, details.formats.data());
  }

  // Presentation Modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    device, m_surface, &presentModeCount, nullptr);
  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

//------------------------------------------------------------------------------
VkSurfaceFormatKHR
Application::chooseSwapSurfaceFormat(
  const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
  for (const auto& availableFormat : availableFormats)
  {
    if (
      availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
      && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      return availableFormat;
    }
  }

  if (availableFormats.empty())
  {
    throw std::runtime_error(
      "no swapchain surface formats available to select");
  }
  return availableFormats[0];
}

//------------------------------------------------------------------------------
VkPresentModeKHR
Application::chooseSwapPresentMode(
  const std::vector<VkPresentModeKHR>& availablePresentModes)
{
  const VkPresentModeKHR requestedMode = VK_PRESENT_MODE_MAILBOX_KHR;
  VkPresentModeKHR bestFallbackMode    = VK_PRESENT_MODE_FIFO_KHR;

  for (const auto& availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == requestedMode)
    {
      return requestedMode;
    }
    else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
    {
      bestFallbackMode = availablePresentMode;
    }
  }

  return bestFallbackMode;
}

//------------------------------------------------------------------------------
VkExtent2D
Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  else
  {
    VkExtent2D actualExtent = {WINDOW_WIDTH, WINDOW_HEIGHT};
    actualExtent.width      = std::max(
      capabilities.minImageExtent.width,
      std::min(capabilities.maxImageExtent.width, actualExtent.width));

    actualExtent.height = std::max(
      capabilities.minImageExtent.height,
      std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
  }
}

//------------------------------------------------------------------------------
void
Application::createImageViews()
{
  m_swapChainImageViews.resize(m_swapChainImages.size());
  for (size_t i = 0; i < m_swapChainImages.size(); ++i)
  {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image                 = m_swapChainImages[i];
    createInfo.viewType              = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format                = m_swapChainImageFormat;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    if (
      vkCreateImageView(
        m_device, &createInfo, nullptr, &m_swapChainImageViews[i])
      != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

//------------------------------------------------------------------------------
void
Application::cleanup()
{
  for (auto imageView : m_swapChainImageViews)
  {
    vkDestroyImageView(m_device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
  vkDestroyDevice(m_device, nullptr);
  if (ENABLE_VALIDATION_LAYERS)
  {
    DestroyDebugUtilsMessengerEXT(m_instance, sg_debugMessenger, nullptr);
  }
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  vkDestroyInstance(m_instance, nullptr);
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

//------------------------------------------------------------------------------
void
Application::init()
{
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
}

//------------------------------------------------------------------------------
void
Application::run()
{
  while (!glfwWindowShouldClose(m_window))
  {
    glfwPollEvents();
  }
}

//------------------------------------------------------------------------------
