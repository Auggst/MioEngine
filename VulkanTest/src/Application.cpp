#include "Application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

EngineCore::MioEngine::MioEngine(){

}

EngineCore::MioEngine::~MioEngine(){

}

/**
 * 运行 EngineCore 的 MioEngine。
 *
 * @throws ErrorType 如果在初始化、主循环或清理过程中出现错误。
 */
void EngineCore::MioEngine::run(){
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

VkResult EngineCore::MioEngine::initWindow(){
    glfwInit(); //初始化GLFW库
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //声明并非创建OpenGL的context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //暂时禁用修改窗口大小

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "MioEnigine", nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

VkResult EngineCore::MioEngine::initVulkan(){
    return VK_SUCCESS;
}

VkResult EngineCore::MioEngine::initInstance(){
    auto result = VK_SUCCESS;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Mio";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MioEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;


    result = vkCreateInstance(&instanceInfo, nullptr, m_instances.data());


    return result;
}

void EngineCore::MioEngine::mainLoop(){
    while(!glfwWindowShouldClose(m_window)){
        glfwPollEvents();
    }
}


void EngineCore::MioEngine::cleanup(){
    //清理GLFW
    glfwDestroyWindow(m_window);
    glfwTerminate();
}