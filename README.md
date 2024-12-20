# Computer Graphics Homework - Particle

| 学号  | **22336090** | **21307210** | **22336180** | **22336223** |
| :---: | ------------ | ------------ | ------------ | ------------ |
| 姓名  | **黄集瑞**   | **傅祉珏**   | **马岱**     | **王晨宇**   |

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

## Attention！！
打开仓库，你会发现有一段的代码实现，但是还有一个压缩包（具体原因在音效添加部分中有解释），如果要运行我们的程序，请将该压缩包直接解压到D盘，然后点进去build，接着是debug，你会发现一个HelloGL.exe，点开这个就可以运行啦！
## Procedure

### 天空盒构建+添加音效（黄集瑞实现）

**【天空盒构建】**
一开始进行天空盒构建时，想要通过HW3来实现，于是变想要考虑利用```box.obj```来进行贴图实现一个天空盒。但是，由于是直接在盒型的建模上进行贴图，所以尝试的贴图的边缘割裂感很强，会一下子让人觉得是处在一个盒子当中而不是一个自然的场景。在该方法解决无果之下，我只好找寻其他搭建天空盒的方法，于是我回到了课程的起点，[LearnOpenGL](https://learnopengl-cn.github.io/04%20Advanced%20OpenGL/06%20Cubemaps/)教程中去搭建天空盒

* 摄像机系统：

  这个实现可以使得我们通过鼠标以及键盘来控制移动以及视角变化，以便于来浏览整个天空盒的搭建。（这部分内容在之前的HW中均有体现）

  ```c++
  void processInput(GLFWwindow *window) {
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
          glfwSetWindowShouldClose(window, true);
  
      float cameraSpeed = 2.5f * deltaTime;
      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
          camera.ProcessKeyboard(FORWARD, deltaTime);
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
          camera.ProcessKeyboard(BACKWARD, deltaTime);
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
          camera.ProcessKeyboard(LEFT, deltaTime);
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
          camera.ProcessKeyboard(RIGHT, deltaTime);
  }
  
  void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
      if (firstMouse) {
          lastX = xpos;
          lastY = ypos;
          firstMouse = false;
      }
  
      float xoffset = xpos - lastX;
      float yoffset = lastY - ypos;
      lastX = xpos;
      lastY = ypos;
  
      camera.ProcessMouseMovement(xoffset, yoffset);
  }
  
  void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
      camera.ProcessMouseScroll(yoffset);
  }
  ```

* 渲染部分：

  渲染的主体共包括两个部分：普通立方体以及天空盒

  这里是使用shader绘制带有纹理的立方体（shader函数也在HW0让我们体验并且实验过）

  ```c++
  
  shader.use();
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
  shader.setMat4("model", model);
  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
  
  // 绘制立方体
  glBindVertexArray(cubeVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, cubeTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  ```

  天空盒渲染：通过移除视角位移部分实现视觉一致性

  ```c++
  skyboxShader.use();
  view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
  skyboxShader.setMat4("view", view);
  skyboxShader.setMat4("projection", projection);
  
  // 绘制天空盒
  glDepthFunc(GL_LEQUAL);
  glBindVertexArray(skyboxVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthFunc(GL_LESS);
  ```

* OpenGL设置

  设置深度测试和VAO/VBO管理

  ```c++
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  // 创建VAO/VBO
  unsigned int cubeVAO, cubeVBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  ```

* Shader使用

  加载和使用着色器，并且传递纹理以及矩阵数据

  ```c++
  Shader shader("path/to/shader.vs", "path/to/shader.fs");
  Shader skyboxShader("path/to/skybox.vs", "path/to/skybox.fs");
  
  // 设置纹理
  shader.use();
  shader.setInt("texture1", 0);
  
  skyboxShader.use();
  skyboxShader.setInt("skybox", 0);
  
  ```

* 加载需要的贴图资源

  分别加载2D纹理以及立方体贴图

  ```c++
  unsigned int loadTexture(char const * path) {
      unsigned int textureID;
      glGenTextures(1, &textureID);
  
      int width, height, nrComponents;
      unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
      if (data) {
          GLenum format = GL_RGB;
          if (nrComponents == 1)
              format = GL_RED;
          else if (nrComponents == 4)
              format = GL_RGBA;
  
          glBindTexture(GL_TEXTURE_2D, textureID);
          glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
          glGenerateMipmap(GL_TEXTURE_2D);
  
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      } else {
          std::cout << "Failed to load texture: " << path << std::endl;
      }
      stbi_image_free(data);
  
      return textureID;
  }
  // -----------------------------------------------------------
  unsigned int loadCubemap(std::vector<std::string> faces) {
      unsigned int textureID;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
  
      int width, height, nrChannels;
      for (unsigned int i = 0; i < faces.size(); i++) {
          unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
          if (data) {
              glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
              stbi_image_free(data);
          } else {
              std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
              stbi_image_free(data);
          }
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  
      return textureID;
  }
  ```

* 渲染循环

  就是在每个HW中我们都可以遇到的主渲染逻辑，可以在这里面处理输入、渲染物体以及实现天空盒

  ```c++
  while (!glfwWindowShouldClose(window)) {
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
  
      processInput(window);
  
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
      // 渲染物体
      shader.use();
      ...
      glBindVertexArray(cubeVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
  
      // 渲染天空盒
      skyboxShader.use();
      ...
      glBindVertexArray(skyboxVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
  
      glfwSwapBuffers(window);
      glfwPollEvents();
  }
  ```

  **【音效添加】**

  在添加音效的时候，也是参考了这个LearnOpenGL教程中的音效实现，选择了```irrklang```库来具体实现。但是在查询用法时才了解到如果使用minggw是无法实现的，必须得用MSVC的编译器才可以使用，而之前的环境以及队员们的都是在我搭建好的vscode框架中写代码的，所以我只好利用vscode重新配MSVC的环境，这也是为什么**仓库里面有一个实现的压缩包，那个压缩包就是我搭建好的环境，主体代码逻辑看仓库上的就可以了，我只是在对应的地方添加上了音效逻辑。**

  * 添加雪花音效：

    由于我的队员实现了雪花的粒子效果，所以我添加了下雪时的音效来模拟对应的下雪。

  ```c++
  // 初始化引擎
  ISoundEngine *SoundEngine = createIrrKlangDevice();
  
  ISound *backgroundmusic = SoundEngine->play2D("D:\\MSVC_ORIGIN\\src\\falling_snow.mp3", GL_TRUE);
  ```

  * 添加烟花爆炸音效：

  ```c++
  // 使用3D音效
  ISound *firework = SoundEngine->play3D("D:\\MSVC_ORIGIN\\src\\firework.mp3", vec3df(pos.x, pos.y, pos.z), false, false, true);
  // 设置音效有效的最近以及最远距离
  firework->setMinDistance(10.f);
  firework->setMaxDistance(35.0f);
  firework->setIsPaused(false);
  // 设置摄像机的原点位置
  SoundEngine->setListenerPosition(vec3df(-5.0f, 0.0f, -5.0f), vec3df(0.0f, 0.0f, -1.0f));
  ```

  一开始本来也是想要使用2D来实现的，但是我们烟花绽放的位置是随机的，所以经常会找不到烟花在哪里绽放，所以我使用了3D音效这样可以达到"听声辨位"，但是目前还是有些问题，还等待者后续提交的再一次优化。

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

`Emit`函数用于初始化烟花爆炸，初始化一个粒子，初始化其位置、速度、颜色和生存时间。这里使用数组中第一个消亡无用的粒子。其中，颜色通过三角函数计算，赋予各维度的值如下x, z方向的基础速度范围是 [-1, 1]，赋予y方向（高度）的基础速度是[-0.7, 1.3]，这里y轴速度相对x, z轴的速度偏正，是因为使更多烟花粒子的初始速度偏向上，因此模拟烟花发射爆炸时向上的初速度。
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


在`main`函数的窗口当中，我们对于每一帧的烟花做了如下的处理。我们首先构建了`framecount`变量，用于计算不同的帧数，以控制烟花绽放的时机。我们仅在`framecount % 300 == 0`时完成烟花的绽放，即每300帧进行一次烟花的绽放。随后我们完成不同位置的随机化。我们使用随机数对烟花的绽放中心进行随机的初始化，但是为了保证烟花每次绽放的位置不一样，我们还对位置信息进行了记录。如果烟花的绽放的位置与上一次的相同，则强行给它换一个绽放中心，这样子就能够确保每次烟花绽放的绽放中心都处于一个不同的位置。最后对颜色进行随机化，即可完成每个粒子的初始化。而为了更好地表示烟花绽放的样子，我们在一帧内一次性生成了1000个粒子，这样子就能模拟烟花在天空中炸开的模样。
```cpp
if (framecount % 300 == 0) {
    switch (rand() % 6) {
        case 0:    center = 0;  break;
        case 1:    center = 1;  break;
        case 2:    center = 2;  break;
        case 3:    center = 3;  break;
        case 4:    center = 4;  break;
        case 5:    center = 5;  break;
        default:    center = 6; break;
    }
    // 保证相邻两次放烟花位置不一样
    if(last_center == center){
        center = (center + 1) % 7;
    }
    last_center = center;
    glm::vec3 pos;
    for (int i = 0; i < 1000; i++) {
        glm::vec3 pos;
        switch (center) {
            case 0: pos = glm::vec3(-9.0f, 6.0f, -9.0f); break;
            case 1: pos = glm::vec3(-4.0f, 6.0f, -4.0f); break;
            case 2: pos = glm::vec3(9.0f, 6.0f, 9.0f); break;
            case 3: pos = glm::vec3(2.0f, 6.0f, 4.0f); break;
            case 4: pos = glm::vec3(7.0f, 6.0f, -7.0f); break;
            case 5: pos = glm::vec3(9.0f, 6.0f, 9.0f); break;
            default: pos = glm::vec3(7.0f, 6.0f, -7.0f); break;
        }

        glm::vec3 color(rand() % 255 / 255.0f, rand() % 255 / 255.0f, rand() % 255 / 255.0f);
        fireworksystem.Emit(pos, color);  
    }

}
fireworksystem.Update(deltaTime);
fireworksystem.Render(fireworkShader, &VAO, &VBO);

framecount += 1;
```
