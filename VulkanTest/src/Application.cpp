#include "Application.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <format>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

/**
 * 一个处理来自Vulkan API的调试消息的回调函数。
 *
 * @param messageSeverity 调试消息的严重程度级别。
 * @param messageType 调试消息的类型。
 * @param pCallbackData 指向调试消息数据的指针。
 * @param pUserData 指向用户数据的指针。
 *
 * @return VK_FALSE 表示调试回调不会中止Vulkan函数调用。
 *
 * @throws 无。
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

EngineCore::MioEngine::MioEngine(){
    m_instances = std::vector<VkInstance>();
    m_instances.reserve(1); //暂时默认只有一个实例
    m_physicalDevices = std::vector<VkPhysicalDevice>();
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
    auto result = VK_SUCCESS;

    //初始化Vulkan实例
    result = initInstance();
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to initialize Vulkan instance.");
    }

    //设置调试信使
    setupDebugMessenger();

    //选取物理设备
    pickPhysicalDevice();

    return result;
}

/**
 * 初始化MioEngine的Vulkan实例。
 *
 * @return VkResult 初始化Vulkan实例的结果。
 *
 * @throws std::runtime_error 如果硬件不支持所需的GLFW扩展。
 * @throws std::runtime_error 如果硬件不支持所需的扩展。
 */
VkResult EngineCore::MioEngine::initInstance() {
    VkResult result = VK_SUCCESS;
    //检查是否支持验证层
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    //创建Vulkan应用程序信息
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Mio",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "MioEngine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    //创建Vulkan实例信息
    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo
    };

    //获取所需的扩展
    auto extensions = getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    //在实例信息里设置验证层
    if (enableValidationLayers) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = nullptr;
    }
    
    //创建Vulkan实例
    result = vkCreateInstance(&instanceInfo, nullptr, m_instances.data());
    
    //检查Vulkan实例是否创建成功
    if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        throw std::runtime_error("Hardware does not support desired extensions.");
    }

    return result;
}

/**
 * 创建一个用于 Vulkan 实例的调试工具信使。
 *
 * @param instance Vulkan 实例。
 * @param pCreateInfo 指向包含调试工具信使创建信息的结构体。
 * @param pAllocator 指向分配器回调结构体的指针。
 * @param pDebugMessenger 指向调试工具信使句柄的指针。
 *
 * @return VkResult 调试工具信使创建的结果。
 *
 * @throws None
 */
VkResult EngineCore::MioEngine::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
 * 获取EngineCore所需的扩展。
 *
 * @return 包含所需扩展的const char*向量。
 *
 * @throws ErrorType 错误描述
 */
std::vector<const char*> EngineCore::MioEngine::getRequiredExtensions(){
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    getSupportedExtensions();

    std::cout << "glfw extension:\n"; 
    for (int i = 0; i < glfwExtensionCount; i++) {
        std::cout << std::format("\t{}\n", glfwExtensions[i]);
        
        bool isSupport = false;
        for (const auto& extension : m_extensions) {
            if (std::string(extension.extensionName) == std::string(glfwExtensions[i])) {
                isSupport = true;
                break;
            }
        }
        
        if (!isSupport) {
            throw std::runtime_error(std::format("Hardware does not support desired GLFW extensions:{}.", glfwExtensions[i]));
        }
    }

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

/**
 * 检查验证层是否支持。
 *
 * @return 如果验证层支持则返回 true，否则返回 false。
 */
bool EngineCore::MioEngine::checkValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers){
        bool layerFound = false;

        for(const auto& layerProperties : availableLayers){
            if (strcmp(layerName, layerProperties.layerName) == 0){
                layerFound = true;
                break;
            }
        }

        if (!layerFound){
            return false;
        }
    }

    return true;
}

/**
 * 获取 EngineCore 类中 MioEngine 的受支持扩展。
 *
 * @return void
 *
 * @throws None
 */
void EngineCore::MioEngine::getSupportedExtensions(){
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    m_extensions = std::vector<VkExtensionProperties>(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_extensions.data());
#if _debug
    std::cout << std::format("available extensions:{}\n", m_extensions.size());
    for (const auto& extension : m_extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
#endif
}

/**
 * 为EngineCore的MioEngine类填充调试信使创建信息。
 *
 * @param createInfo 要填充的VkDebugUtilsMessengerCreateInfoEXT对象的引用。
 *
 * @return void
 *
 * @throws None
 */
void EngineCore::MioEngine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
}

/**
 * 设置调试信使函数，用于在EngineCore类中的MioEngine中设置调试消息处理。
 * 该函数负责创建Vulkan API中处理调试消息所需的结构体和回调函数。
 *
 * @throws std::runtime_error 如果调试信使设置失败。
 */
void EngineCore::MioEngine::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    populateDebugMessengerCreateInfo(createInfo);

    for(const auto& instance: m_instances){
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
}


/**
 * 销毁调试工具信使扩展。
 *
 * @param instance Vulkan实例。
 * @param debugMessenger 要销毁的调试信使。
 * @param pAllocator 分配回调。
 *
 * @throws ErrorType 错误的描述。
 */
void EngineCore::MioEngine::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


void EngineCore::MioEngine::pickPhysicalDevice(){
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instances.front(), &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    //遍历所有可用的GPU后，写入m_physicalDevices
    m_physicalDevices.reserve(deviceCount);
    vkEnumeratePhysicalDevices(m_instances.front(), &deviceCount, m_physicalDevices.data());

    for(const auto& device: m_physicalDevices){
        if (isDeviceSuitable(device)){
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool isDeviceSuitable(VkPhysicalDevice device){
    return true;
}

void EngineCore::MioEngine::mainLoop(){
    while(!glfwWindowShouldClose(m_window)){
        glfwPollEvents();
    }
}

void EngineCore::MioEngine::cleanup(){
    //清理Vulkan实例
    if (!m_instances.empty()){
        for (const auto& instance : m_instances){
            if (enableValidationLayers) {
                //清理调试信使    
                DestroyDebugUtilsMessengerEXT(m_instances.front(), m_debugMessenger, nullptr);  
            }
            
            //清理Vulkan实例
            vkDestroyInstance(instance, nullptr);
        }
        m_instances.clear();
    }

    //清理GLFW
    glfwDestroyWindow(m_window);
    glfwTerminate();
}