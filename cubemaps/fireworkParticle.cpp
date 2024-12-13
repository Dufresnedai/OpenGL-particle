#include <vector>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <learnopengl/shader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <learnopengl/camera.h>

struct FireworkParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float lifetime;
    float size;
    std::vector<glm::vec3> historyPositions;  
    void reset() {
        position = glm::vec3(0.0f, 0.0f, 0.0f);
        velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        lifetime = -1.0f;
        size = 1.0f;
        historyPositions.clear();  
    }
};

class FireworkSystem {
public:
    int maxParticles;
    std::vector<FireworkParticle> particles;
    unsigned int texture;
    FireworkSystem(int maxParticles) : maxParticles(maxParticles), particles(maxParticles) {
        for (int i = 0; i < maxParticles; ++i) {
            particles[i].lifetime = -1.0f;
        }

        GLfloat pointSizeRange[2];
        glGetFloatv(GL_POINT_SIZE_RANGE, pointSizeRange);
        std::cout << "Particle size range: " << pointSizeRange[0] << " - " << pointSizeRange[1] << std::endl;

        if (pointSizeRange[0] <= 0.0f || pointSizeRange[1] <= 0.0f) {
            pointSizeRange[0] = 1.0f;
            pointSizeRange[1] = 1000.0f;
        }
    }

    void Emit(const glm::vec3& position, const glm::vec3& color) {
        for (int i = 0; i < maxParticles; ++i) {
            if (particles[i].lifetime <= 0.0f) {
                particles[i].position = position;
                float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * glm::pi<float>();
                float phi = static_cast<float>(rand()) / RAND_MAX * glm::pi<float>();
                float x = sin(phi) * cos(theta);
                float y = sin(phi) * sin(theta);
                float z = cos(phi);
                particles[i].velocity = glm::vec3(x, y, z) * (0.5f + static_cast<float>(rand()) / RAND_MAX * 40.0f);

                particles[i].color = color;
                particles[i].lifetime = 0.625f;
                break;
            }
        }
    }

    void Update(float deltaTime) {
        for (int i = 0; i < maxParticles; ++i) {
            if (particles[i].lifetime > 0.0f) {
                particles[i].lifetime -= deltaTime;
                // 假设粒子的加速度为重力加速度
                glm::vec3 gravity(0.0f, -3.0f, 0.0f); // 重力加速度，向下
                particles[i].velocity += gravity * deltaTime;  // 速度受重力影响
                particles[i].velocity *= 0.98;
                particles[i].position += particles[i].velocity * deltaTime;
                particles[i].size *= 0.99f;          

                // 更新粒子的拖尾位置
                particles[i].historyPositions.push_back(particles[i].position);
                if (particles[i].historyPositions.size() > 10) {  
                    particles[i].historyPositions.erase(particles[i].historyPositions.begin());
                }
            }
            else{
                particles[i].reset();
            }
        }
    }

    std::string tostring(glm::vec3 vec) {
        std::stringstream string;
        string << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return string.str();
    }

    void Render(GLuint shaderProgram, GLuint VAO, GLuint VBO) {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> colors;
        std::vector<float> sizes;
        std::vector<float> alphaValues;  // 用来存储透明度
        for (int i = 0; i < maxParticles; ++i) {
            if (particles[i].lifetime > 0.0f) {
                positions.push_back(particles[i].position);
                // std::cout << "Positions: " << tostring(particles[i].position) << std::endl;
                colors.push_back(particles[i].color);
                // std::cout << "Color: " << tostring(particles[i].color) << std::endl;
                sizes.push_back(particles[i].size);
                // std::cout << "Sizes: " << particles[i].size << std::endl;
                // 渲染粒子的拖尾
                alphaValues.push_back(1.0f); // 粒子本身的透明度
                for (size_t j = 0; j < particles[i].historyPositions.size(); ++j) {
                    positions.push_back(particles[i].historyPositions[j]);
                    colors.push_back(particles[i].color);
                    sizes.push_back(particles[i].size * 0.5f); 
                    alphaValues.push_back(1.0f - static_cast<float>(j) / particles[i].historyPositions.size());  
                }
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (positions.size() + colors.size() + sizes.size()) * sizeof(glm::vec3), nullptr, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
        glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), colors.size() * sizeof(glm::vec3), colors.data());
        glBufferSubData(GL_ARRAY_BUFFER, (positions.size() + colors.size()) * sizeof(glm::vec3), sizes.size() * sizeof(float), sizes.data());
        glBufferSubData(GL_ARRAY_BUFFER, (positions.size() + colors.size() + sizes.size()) * sizeof(float), alphaValues.size() * sizeof(float), alphaValues.data());
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, positions.size());
    }
};