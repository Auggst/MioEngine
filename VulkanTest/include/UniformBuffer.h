#pragma once

#include <vector>
#include <array>

#include <glm/glm.hpp>

namespace EngineCore{
struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
}
