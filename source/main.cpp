#include <fmt/format.h>
#include <vector>
#include <map>
#include <set>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
//------------------------------------------------------------------------------
constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

const std::vector<const char*> VALIDATION_LAYERS
  = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

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
static VkDebugUtilsMessengerEXT sg_debugMessenger;

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
VkResult
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
void
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
void
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
void
setupDebugMessenger(VkInstance& instance)
{
  assert(ENABLE_VALIDATION_LAYERS);

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  if (
    CreateDebugUtilsMessengerEXT(
      instance, &createInfo, nullptr, &sg_debugMessenger)
    != VK_SUCCESS)
  {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

//------------------------------------------------------------------------------
bool
checkValidationLayerSupport()
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
getRequiredExtensions()
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
void
createInstance(VkInstance& instance)
{
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

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
  {
    throw std::runtime_error("Couldn't create VkInstance!");
  }
}

//------------------------------------------------------------------------------
QueueFamilyIndices
findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
{
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
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
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
int
rateDeviceSuitability(
  const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
{
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

  // Hard requirements
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
  if (!indices.isComplete())
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
VkPhysicalDevice
pickPhysicalDevice(VkInstance& instance, const VkSurfaceKHR& surface)
{
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0)
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  std::multimap<int, VkPhysicalDevice> devicesByScore;
  for (const auto& device : devices)
  {
    int score = rateDeviceSuitability(device, surface);
    devicesByScore.insert(std::make_pair(score, device));
  }

  if (!devicesByScore.empty() && devicesByScore.rbegin()->first > 0)
  {
    physicalDevice = devicesByScore.rbegin()->second;
  }
  else
  {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  return physicalDevice;
}

//------------------------------------------------------------------------------
void
createLogicalDevice(
  VkDevice& device,
  VkQueue& graphicsQueue,
  VkQueue& presentQueue,
  const VkPhysicalDevice& physicalDevice,
  const VkSurfaceKHR& surface)
{
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

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
  createInfo.enabledExtensionCount = 0;
  if (ENABLE_VALIDATION_LAYERS)
  {
    createInfo.enabledLayerCount
      = static_cast<uint32_t>(VALIDATION_LAYERS.size());
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  }

  if (
    vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

//------------------------------------------------------------------------------

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

  //----------------------------------------------------------------------------
  void cleanup()
  {
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

  //----------------------------------------------------------------------------
  void createSurface()
  {
    if (
      glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface)
      != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create window surface!");
    }
  }

  //----------------------------------------------------------------------------

public:
  ~Application() { cleanup(); }

  //----------------------------------------------------------------------------
  void init()
  {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);    // not using OpenGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);      // not resizable

    m_window = glfwCreateWindow(
      WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan hello triangle", nullptr, nullptr);

    createInstance(m_instance);
    if (ENABLE_VALIDATION_LAYERS)
    {
      setupDebugMessenger(m_instance);
    }
    createSurface();
    m_physicalDevice = pickPhysicalDevice(m_instance, m_surface);
    createLogicalDevice(m_device, m_graphicsQueue, m_presentQueue, m_physicalDevice, m_surface);
  }

  //----------------------------------------------------------------------------
  void run()
  {
    while (!glfwWindowShouldClose(m_window))
    {
      glfwPollEvents();
    }
  }
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
int
main()
{
  try
  {
    Application app;
    app.init();

    app.run();
  }
  catch (const std::exception& e)
  {
    fmt::print("{}\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
