#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace EngineCore{
struct MioInfo{
    std::string name;
};

class MioEngine{
//函数
public:
    MioEngine();
    ~MioEngine();
    void run();
    VkResult init();

private:
    VkResult initWindow();
    VkResult initVulkan();
    VkResult initInstance();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger); 
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    void getSupportedExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void mainLoop();
    void cleanup();
//字段
private:
    GLFWwindow* m_window;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    std::vector<VkInstance> m_instances;
    std::vector<VkExtensionProperties> m_extensions;
    std::vector<VkPhysicalDevice> m_physicalDevices; 
    std::vector<VkDevice> m_logicalDevice;

};

}