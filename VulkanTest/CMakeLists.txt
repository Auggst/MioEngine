cmake_minimum_required(VERSION 3.15)  # 设置CMake的最低版本要求

project(MioEngine VERSION 1.0)  # 定义项目名称和版本

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    # 设置MSVC编译器使用UTF-8编码
    add_compile_options("/source-charset:utf-8")
endif()

# 找到Vulkan SDK的路径，CMake通常可以自动找到
find_package(Vulkan REQUIRED)

add_executable(MioEngine)

# 添加子目录
add_subdirectory(src)

# 添加源文件
target_sources(MioEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# 添加include目录
target_include_directories(MioEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 如果有外部库，链接它们
# target_link_libraries(MyModernProject PRIVATE some_library)
# 链接Vulkan库
target_link_libraries(MioEngine PRIVATE Vulkan::Vulkan)

# 链接GLFW库
## 指定GLFW库的位置
set(GLFW_LIBRARY "F:/EnvironmentDev/glfw-3.3.8/glfw-3.3.8.bin.WIN64/lib-vc2022/glfw3.lib")
target_link_libraries(MioEngine PRIVATE ${GLFW_LIBRARY})