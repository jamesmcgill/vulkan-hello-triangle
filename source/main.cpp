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

//------------------------------------------------------------------------------
VkResult
createInstance(VkInstance& instance)
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions
    = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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
  for (uint32_t i = 0; i < glfwExtensionCount; ++i)
  {
    fmt::print("\t {}\n", glfwExtensions[i]);
  }

  VkApplicationInfo appInfo  = {};
  appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName   = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName        = "No Engine";
  appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion         = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo    = {};
  createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo        = &appInfo;
  createInfo.enabledExtensionCount   = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;
  createInfo.enabledLayerCount       = 0;

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

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
  }

cleanup:
  vkDestroyInstance(instance, nullptr);
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
