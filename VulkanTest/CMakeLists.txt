cmake_minimum_required(VERSION 3.24)  # 设置CMake的最低版本要求

project(MioEngine VERSION 1.0)  # 定义项目名称和版本

# 设置C++标准为C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    # 设置MSVC编译器使用UTF-8编码
    add_compile_options("/source-charset:utf-8")
endif()

# 如果没有指定构建类型，则默认设置为Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 假设 GLFW 和其他依赖库的头文件和库文件都在项目目录下的 env 文件夹中
set(ENV_DIR ${CMAKE_SOURCE_DIR}/Env)

# 找到Vulkan SDK的路径，CMake通常可以自动找到
find_package(Vulkan REQUIRED)

add_executable(MioEngine)

# 添加子目录
add_subdirectory(src)

# 添加源文件
target_sources(MioEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
# 添加环境include目录
target_include_directories(MioEngine PRIVATE ${ENV_DIR}/include)
# 添加本地include目录
target_include_directories(MioEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 如果有外部库，链接它们
# target_link_libraries(MyModernProject PRIVATE some_library)
# 链接Vulkan库
target_link_libraries(MioEngine PRIVATE Vulkan::Vulkan)

# 链接GLFW库
## 指定GLFW库的位置
set(GLFW_LIBRARY "${ENV_DIR}/lib/glfw3.dll")
target_link_libraries(MioEngine PRIVATE "${ENV_DIR}/lib/glfw3.dll" "${ENV_DIR}/lib/libglfw3.a" "${ENV_DIR}/lib/libglfw3dll.a")

# # 链接Assimp库
# set(ASSIMP_LIBRARY "${ENV_DIR}/lib/assimp-vc143-mt.lib")
# target_link_libraries(MioEngine PRIVATE ${ASSIMP_LIBRARY})