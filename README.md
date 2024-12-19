# Computer Graphics Homework - Particle

| 学号 | **22336090** | **21307210** | **22336180** | **22336223** |
| :--: | ------------------ | ------------------ | ------------------ | ------------------ |
| 姓名 | **黄集瑞**   | **傅祉珏**   | **马岱**     | **王晨宇**   |

## Introduction

This is our group's final assignment for computer graphics. We completed the design and implementation of a particle system based on OpenGL. Specifically, we implemented snowflake particles and firework particles in the skybox. The following is a brief description of the experiment.

## Requirements

这个选题主要是关于粒子特效的构建方面，你可以构建一个三维的粒子系统，也可以构建一个二维的粒子系统（使用的渲染库不限，但不得直接套用游戏引擎库的粒子系统，使用游戏引擎的话，粒子系统必须是自己构建的）。在本选题中，你将构建一个烟花的粒子系统，烟花的类型由你自己定，参考实现的功能：

* 构建烟花粒子模型：不要求你使用GPU加速，你只需在CPU上实现烟花粒子系统即可；
* 添加天空盒：添加一个黑夜的天空盒作为背景；
* 添加一个地面：为了使得场景不那么突兀，你只需添加一个平面作为地面；
* 实现烟花光照：为了实现烟花爆炸瞬间的光辉璀璨，你需要在每朵烟花的爆炸中心激活一个点光源，将该点光源的颜色选定为烟花的颜色，爆炸结束后再删除这个点光源。地面需要实现光照（传统Blinn-Phong光照即可），从而凸显烟火闪烁的效果；
* 实现辉光特效：使用高斯模糊之类的后处理技术实现烟花周围的光晕特效；
* 添加音效：在烟花爆炸的时候添加一个Boom的音效。

## Procedure

### （黄集瑞实现）

### 窗口调整+光标设计+雪花粒子实现（马岱实现）

**【窗口调整】**
为了能够更清楚的展示之后的粒子效果，我们完成了窗口实现自适应屏幕的设计，这里主要是插入一个按键回调函数，使用GLFW库在按下 F 键时在全屏和窗口模式之间切换。
全屏切换逻辑如下：

* 进入全屏模式：
  - 使用 glfwGetPrimaryMonitor 获取主显示器。
  - 获取显示器的分辨率和刷新率 (glfwGetVideoMode)。
  - 记录当前窗口模式下的位置 (glfwGetWindowPos) 和大小 (glfwGetWindowSize)。
  - 调用 glfwSetWindowMonitor 切换到全屏模式。
* 返回窗口模式：
  - 使用先前记录的窗口位置和大小。
  - 通过 glfwSetWindowMonitor 将窗口返回到非全屏模式。

```cpp
// 按键回调函数
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    { // 按下 F 键切换模式
        isFullscreen = !isFullscreen;
        if (isFullscreen)
        {
            // 切换到全屏模式
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();      // 获取主显示器
            const GLFWvidmode *mode = glfwGetVideoMode(monitor); // 获取显示器模式
            // 记录窗口模式下的位置和大小
            glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
            glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
            // 设置全屏模式
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            // 切换回窗口模式
            glfwSetWindowMonitor(window, nullptr, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
        }
    }
}
```

**【光标设计】**
接着为了使光标更优美，我们又优化了原有的鼠标光标，通过 glfwSetCursor 设置自定义光标，使其在窗口中显示。
![mc](cubemaps/resources/textures/skybox/README/readme_image1.png)

```cpp
GLFWcursor *createCursor(const char *imagePath)
{
    int width, height, channels;
    unsigned char *data = stbi_load(imagePath, &width, &height, &channels, 0);
    if (!data)
    {
        std::cout << "Failed to load cursor image!" << std::endl;
        return nullptr;
    }

    GLFWimage cursorImage = {width, height, data};
    GLFWcursor *cursor = glfwCreateCursor(&cursorImage, 0, 0);

    stbi_image_free(data); // 释放图像数据
    return cursor;
}
```

```cpp
    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // 显示光标
    // glfwSetCursor(window, glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR)); // 十字光标

    // 加载自定义光标
    GLFWcursor *customCursor = createCursor("D:\\OpenGL\\particle\\OpenGLparticle\\cubemaps\\resources\\textures\\skybox\\mc_j.png");

    if (customCursor)
    {
        glfwSetCursor(window, customCursor); // 设置自定义光标
    }
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
```

**【雪花粒子实现】**

![snow](cubemaps/resources/textures/skybox/README/readme_image2.png)

为了使整个氛围更有意境，我们又在烟花粒子的基础上实现了雪花粒子的插入，具体的设计和操作如下：

* 雪花粒子：
  - snowParticle 存储粒子位置和速度。
  - 粒子位置为 3D 坐标，速度控制粒子下落的快慢。

```cpp
struct snowParticle {
    glm::vec3 position;  // 粒子的位置
    float speed;         // 粒子的下落速度
};
```

* 雪花系统类 SnowSystem：创建大量的粒子，并赋予每个粒子随机的初始位置和速度，构建雪花系统的基本状态。模拟雪花的下落过程，并在粒子越界时重置其位置，保持粒子系统的连续性。
  - 粒子初始化：生成一定数量的粒子，并为每个粒子随机设置初始位置和速度。
  - 更新逻辑：模拟粒子下落，超出屏幕范围后重置到顶部。每个粒子在每帧下落的距离与其速度和时间步长（`deltaTime`）成正比。如果粒子的 Y 坐标低于一定阈值（如 `-50.0f`），则重置其位置至顶部。在重置位置时重新生成 X 和 Z 坐标，确保新粒子分布随机性。
  - OpenGL 初始化：绑定 VAO、VBO，并存储粒子的位置数据。
  - 渲染逻辑：更新粒子位置数据并绘制为点图元。每帧更新 VBO 数据，确保粒子位置的实时性。使用着色器设置投影、视图和模型矩阵，实现粒子的空间变换。将粒子渲染为点，并绑定纹理，模拟雪花效果。使用 `glBufferSubData` 将新的粒子位置传输到 GPU。通过 `glUniformMatrix4fv` 将投影和视图矩阵传递给着色器。通过绑定纹理，将粒子渲染为雪花形状。

```cpp
SnowSystem() {
    srand(static_cast<unsigned int>(time(0))); // 初始化随机数种子
    for (unsigned int i = 0; i < NUM_PARTICLES; ++i) {
        particles.push_back({
            glm::vec3((rand() % 201 - 100) / 1.0f, 50.0f, 
                    (rand() % 201 - 100) / 1.0f),   // 随机位置
            0.001f + (rand() % 100) / 3.0f         // 随机速度
        });
    }
}

void updateParticles(float deltaTime) {
    for (auto &particle : particles) {
        particle.position.y -= particle.speed * deltaTime; // 按速度下落
        if (particle.position.y < -50.0f) { // 超出范围时重置
            particle.position.y = 100.0f;   // 回到顶部
            particle.position.x = (rand() % 201 - 100) / 1.0f; // 随机 X
            particle.position.z = (rand() % 201 - 100) / 1.0f; // 随机 Z
        }
    }
}

void render(unsigned int shaderProgram, glm::mat4 projection, glm::mat4 view) {
    std::vector<float> positions;
    for (const auto &particle : particles) {
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
```

* 着色器：
  - 顶点着色器：处理粒子在 3D 世界中的位置及点大小。
  - 片段着色器：为粒子绘制纹理。

```cpp
const char *snowvs = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = 40.0; 
}
)";

const char *snowfs = R"(
#version 330 core
out vec4 FragColor;
uniform sampler2D snowflakeTexture;
void main()
{
    FragColor = texture(snowflakeTexture, gl_PointCoord);
}
)";
```

---

### 烟花实现（王晨宇 && 傅祉珏实现）

**【烟花粒子结构体】**

一个烟花粒子结构体如下：
```cpp
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
```
其中，各变量成员含义为：

- position：烟花粒子的初始位置
- velocity：烟花粒子的速度
- color：烟花粒子的颜色
- lifetime：粒子的生存时间，当其小于等于0时，烟花粒子消失
- size：烟花粒子的大小，这里也可以在着色器直接设置
- historyPositions：用于记录烟花历史位置的数组，用于实现烟花粒子拖尾效果
  

**【烟花系统类】**

创建烟花系统类，便于管理所有烟花粒子，其定义如下：
```cpp
class FireworkSystem {
public:
    int maxParticles;
    std::vector<FireworkParticle> particles;
    FireworkSystem(int maxParticles);
    void Emit(const glm::vec3& position, const glm::vec3& color)；
    void Update(float deltaTime)；
    void Render(GLuint shaderProgram, GLuint* VAO, GLuint* VBO);
};
```

`Emit`函数用于初始化烟花爆炸，初始化一个粒子，初始化其位置、速度、颜色和生存时间。这里舒勇数组中第一个消亡无用的粒子。其中，颜色通过三角函数计算，赋予各维度的值如下x, z方向的基础速度范围是 [-1, 1]，赋予y方向（高度）的基础速度是[-0.7, 1.3]，这里y轴速度相对x, z轴的速度偏正，是因为使更多烟花粒子的初始速度偏向上，因此模拟烟花发射爆炸时向上的初速度。
```cpp
void Emit(const glm::vec3& position, const glm::vec3& color) {
    for (int i = 0; i < maxParticles; ++i) {
        if (particles[i].lifetime <= 0.0f) {
            particles[i].position = position;
            float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * glm::pi<float>();
            float phi = static_cast<float>(rand()) / RAND_MAX * glm::pi<float>();
            float x = sin(phi) * cos(theta);
            float y = sin(phi) * sin(theta) + 0.3f; 
            float z = cos(phi);
            particles[i].velocity = glm::vec3(x, y, z) * (1.0f + static_cast<float>(rand()) / RAND_MAX * 40.0f);
            particles[i].color = color;
            particles[i].lifetime = 0.8f + (rand() % 10) / 20.0f;
            break;
        }
    }
}
```

`Update`函数用于每一帧更新粒子信息，如果粒子生存时间小于等于0，则重置粒子；否则，更新粒子信息，这里包括更新粒子速度，速度会逐渐减小，此外，为了模拟重力作用，y轴速度还会额外减小一部份。此外还要更新粒子的历史位置，更新粒子的历史位置，这里允许最大存储数量为40，当超过这个数目时，需要替代最旧的历史位置。
```cpp
void Update(float deltaTime) {
    for (int i = 0; i < maxParticles; ++i) {
        if (particles[i].lifetime > 0.0f) {
            particles[i].lifetime -= deltaTime;
            // 假设粒子的加速度为重力加速度
            glm::vec3 gravity(0.0f, -20.0f, 0.0f); // 重力加速度，向下
            particles[i].velocity += gravity * deltaTime;  // 速度受重力影响
            particles[i].velocity *= 0.98;
            particles[i].position += particles[i].velocity * deltaTime;
            particles[i].size *= 0.99f;          

            // 更新粒子的拖尾位置
            particles[i].historyPositions.push_back(particles[i].position);
            if (particles[i].historyPositions.size() > 40) {  
                particles[i].historyPositions.erase(particles[i].historyPositions.begin());
            }
        }
        else{
            particles[i].reset();
        }
    }
}
```

`Render`渲染一个粒子效果，显示粒子的变化。该函数对于实现效果较为重要的部分是粒子透明度的变化，每一个粒子历史位置都对应着一个透明度（即$\alpha$通道），当粒子越旧，$\alpha$ 越接近0，其就越透明；当粒子越新，$\alpha$ 越接近1，其就越不透明。这样实现的粒子效果更加接近现实。
```cpp
void Render(GLuint shaderProgram, GLuint* VAO, GLuint* VBO) {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> colors;
    std::vector<float> sizes;
    std::vector<float> alphaValues;  // 用来存储透明度
    for (int i = 0; i < maxParticles; ++i) {
        if (particles[i].lifetime > 0.0f) {
            positions.push_back(particles[i].position);
            colors.push_back(particles[i].color);
            sizes.push_back(particles[i].size);
            alphaValues.push_back(1.0f); 
            for (size_t j = 0; j < particles[i].historyPositions.size(); ++j) {
                positions.push_back(particles[i].historyPositions[j]);
                colors.push_back(particles[i].color);
                sizes.push_back(particles[i].size * 0.9f); 
                alphaValues.push_back(1.0f - 0.5f * static_cast<float>(j) / particles[i].historyPositions.size());  
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    size_t totalSize = positions.size() * sizeof(glm::vec3) + 
                    colors.size() * sizeof(glm::vec3) + 
                    sizes.size() * sizeof(float) + 
                    alphaValues.size() * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
    glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), colors.size() * sizeof(glm::vec3), colors.data());
    glBufferSubData(GL_ARRAY_BUFFER, (positions.size() + colors.size()) * sizeof(glm::vec3), sizes.size() * sizeof(float), sizes.data());
    glBufferSubData(GL_ARRAY_BUFFER, (positions.size() + colors.size() + sizes.size()) * sizeof(float), alphaValues.size() * sizeof(float), alphaValues.data());
    glUseProgram(shaderProgram);
    glBindVertexArray(*VAO);
    glDrawArrays(GL_POINTS, 0, positions.size());
}
```