#include <vector>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <learnopengl/shader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <learnopengl/camera.h>
#include <glm/gtc/type_ptr.hpp>

struct snowParticle
{
    glm::vec3 position;
    float speed;
};

class SnowSystem {
public:
    const unsigned int NUM_PARTICLES = 4000;
    std::vector<snowParticle> particles;
    unsigned int VAO, VBO, texture;

    // 初始化粒子位置和速度
    SnowSystem()
    {
        srand(static_cast<unsigned int>(time(0)));
        for (unsigned int i = 0; i < NUM_PARTICLES; ++i)
        {
            // x, z: [-50.0, 50.0] 随机
            // y
            particles.push_back({
                glm::vec3((rand() % 201 - 100) / 1.0f, 
                          (rand() % 201 - 50) / 1.0f, 
                          (rand() % 201 - 100) / 1.0f),   
                8.0f + (rand() % 4) / 3.0f      // 控制速度
            });
        }
    }

    // 更新粒子位置，超出范围则重置位置
    void updateParticles(float deltaTime)
    {
        for (auto &particle : particles)
        {
            particle.position.y -= particle.speed * deltaTime; 
            // 粒子下落到屏幕底部时重置
            if (particle.position.y < -50.0f) 
            {
                particle.position.y = 100.0f; 
                particle.position.x = (rand() % 201 - 100) / 1.0f; 
                particle.position.z = (rand() % 201 - 100) / 1.0f; 
            }
        }
    }
    // 初始化OpenGL
    void initOpenGL()
    {
        std::vector<float> positions;
        for (const auto &particle : particles)
        {
            positions.push_back(particle.position.x);
            positions.push_back(particle.position.y);
            positions.push_back(particle.position.z);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
    void render(unsigned int shaderProgram, glm::mat4 projection, glm::mat4 view){
        std::vector<float> positions;
        for (const auto &particle : particles)
        {
            positions.push_back(particle.position.x);
            positions.push_back(particle.position.y);
            positions.push_back(particle.position.z);
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(float), positions.data());
        glUseProgram(shaderProgram);

        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");

        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "snowflakeTexture"), 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
        glBindVertexArray(0);
    }
};
