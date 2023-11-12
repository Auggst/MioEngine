# src/CMakeLists.txt
set(SOURCE_FILES
    main.cpp
    Application.cpp
)

# 可以选择添加当前源文件到父级目标
target_sources(MioEngine PRIVATE ${SOURCE_FILES})