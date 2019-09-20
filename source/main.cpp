#include <fmt/format.h>
#include <vector>

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
VkResult
setupDebugMessenger(VkInstance& instance)
{
  assert(ENABLE_VALIDATION_LAYERS);

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  return CreateDebugUtilsMessengerEXT(
    instance, &createInfo, nullptr, &sg_debugMessenger);
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
VkResult
createInstance(VkInstance& instance)
{
  if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
  {
    return VK_ERROR_LAYER_NOT_PRESENT;
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

  return vkCreateInstance(&createInfo, nullptr, &instance);
}

//------------------------------------------------------------------------------
int
main()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);    // not using OpenGL
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);      // not resizable
  GLFWwindow* window = glfwCreateWindow(
    WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan hello triangle", nullptr, nullptr);

  VkInstance instance;
  if (createInstance(instance) != VK_SUCCESS)
  {
    fmt::print("Couldn't create VkInstance. Terminating\n");
    goto cleanup;
  }

  if (ENABLE_VALIDATION_LAYERS && setupDebugMessenger(instance) != VK_SUCCESS)
  {
    fmt::print("failed to set up debug messenger. Terminating\n");
    goto cleanup;
  }

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
  }

cleanup:
  if (ENABLE_VALIDATION_LAYERS)
  {
    DestroyDebugUtilsMessengerEXT(instance, sg_debugMessenger, nullptr);
  }
  vkDestroyInstance(instance, nullptr);
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
