#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <optional>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace EngineCore{
struct MioInfo{
    std::string name;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily = -1;
    std::optional<uint32_t> presentFamily = -1;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class MioEngine{
//函数
public:
    MioEngine();
    ~MioEngine();
    void run();
private:
    VkResult initWindow();
    VkResult initVulkan();
    void createInstance();
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger); 
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamily(VkPhysicalDevice device);
    void getSupportedExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void mainLoop();
    void cleanup();
//字段
private:
    GLFWwindow* m_window;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkSurfaceKHR m_surface;
    VkQueue presentQueue;
    std::vector<VkInstance> m_instances;
    std::vector<VkExtensionProperties> m_extensions;
    std::vector<VkPhysicalDevice> m_physicalDevices; 
    std::vector<VkDevice> m_logicalDevice;
    std::vector<VkQueue> m_graphicsQueue;

};

}