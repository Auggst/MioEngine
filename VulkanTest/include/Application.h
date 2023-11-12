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
    void mainLoop();
    void cleanup();
//字段
private:
    GLFWwindow* m_window;
    std::vector<VkInstance> m_instances;
    std::vector<VkPhysicalDevice> m_physicalDevices; 
    std::vector<VkDevice> m_logicalDevice;

};

}