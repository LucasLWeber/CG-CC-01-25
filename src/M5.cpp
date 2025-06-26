#include <iostream>
#include <string>
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

struct Geometry
{
    GLuint VAO;
    GLuint vertexCount;
    GLuint textureID = 0;
    string textureFilePath;
    glm::vec3 position;
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec3 ke;
    float scaleFactor = 0.4;
    float shininess = 32.0f;
};

struct Material
{
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec3 ke;
    float shininess;
    string texturePath;
};

struct Camera
{
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float Yaw;
    float Pitch;
    float Speed;
    float Sensitivity;
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), Speed(1.0f), Sensitivity(0.1f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }
    void ProcessKeyboard(const string& direction, float deltaTime)
    {
        float velocity = Speed * deltaTime;

        if (direction == "FORWARD")
        {
            Position += Front * velocity;
        }
        if (direction == "BACKWARD")
        {
            Position -= Front * velocity;
        }
        if (direction == "LEFT")
        {
            Position -= Right * velocity;
        }
        if (direction == "RIGHT")
        {
            Position += Right * velocity;
        }
    }
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void continous_key_press(GLFWwindow* window, Camera& camera, float currentTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
int setupShader();
Geometry setupGeometry(const char* filepath);
bool loadObject(
    const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals);
int loadTexture(const string& path);
Material loadMTL(const string& path);

const GLchar *vertexShaderSource = R"(
	#version 400
	layout (location = 0) in vec3 position;
	layout (location = 1) in vec2 tex_coord;
	layout (location = 2) in vec3 normal;
	out vec2 texCoord;
	out vec3 fragPos;
	out vec3 fragNormal;
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;
	void main()
	{
			vec4 worldPos = model * vec4(position, 1.0);
			gl_Position = projection * view * worldPos;
			texCoord = vec2(tex_coord.x, 1.0 - tex_coord.y);
			fragPos = vec3(worldPos);
			fragNormal = mat3(transpose(inverse(model))) * normal;
	}
)";

const GLchar *fragmentShaderSource = R"(
	#version 400
	in vec3 fragNormal;
	in vec3 fragPos;
	in vec2 texCoord;
	uniform vec3 ka;
	uniform vec3 kd;
	uniform vec3 ks;
	uniform float q;
	uniform vec3 lightPos;
	uniform vec3 lightColor;
	uniform vec3 cameraPos;
	uniform sampler2D colorBuffer;
	out vec4 color;
	void main()
	{
			vec3 N = normalize(fragNormal);
			vec3 L = normalize(lightPos - fragPos);
			vec3 V = normalize(cameraPos - fragPos);
			vec3 R = reflect(-L, N);
			float distance = length(lightPos - fragPos);
			float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
			vec3 ambient = ka * lightColor * 0.05;
			float diff = max(dot(N, L), 0.0);
			vec3 texColor = texture(colorBuffer, texCoord).rgb;
			vec3 diffuse = diff * kd * lightColor * texColor * attenuation * 3;
			vec3 specular = vec3(0.0);
			if (diff > 0.0)
			{
					float spec = pow(max(dot(R, V), 0.0), q);
					specular = spec * ks * lightColor * attenuation * 2.0;
			}
			vec3 result = ambient + diffuse + specular;
			color = vec4(result, 1.0);
	}
)";

const GLuint WIDTH = 600, HEIGHT = 600;
bool rotateX=true, rotateY=false, rotateZ=false;
glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
string mtlFilePath = "";
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 15.0f); 
glm::vec3 ambientColor(0.2f), diffuseColor(0.2f), specularColor(1.5f), emissiveColor(1.0f);
Camera camera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	-90.0f,
	0.0f
);
bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Lucas Weber!", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported: " << glGetString(GL_VERSION) << std::endl;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    GLuint shaderID = setupShader();
    glUseProgram(shaderID);
    Geometry geometry = setupGeometry("D:/ComputacaoGrafica/RepoAulas/repo-prof/CGCCHibrido/assets/Modelos3D/SuzanneSubdiv1.obj");
    geometry.position = glm::vec3(0.0f, 0.0f, -2.0f); 
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, geometry.position);
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc  = glGetUniformLocation(shaderID, "view");
    GLint projLoc  = glGetUniformLocation(shaderID, "projection");
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        continous_key_press(window, camera, deltaTime);
        view = camera.GetViewMatrix();
				model = glm::mat4(1.0f);
				model = glm::translate(model, geometry.position);
				model = glm::scale(model, glm::vec3(geometry.scaleFactor));
				if (rotateX) model = glm::rotate(model, currentFrame, glm::vec3(1.0f, 0.0f, 0.0f));
				else if (rotateY) model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 1.0f, 0.0f));
				else if (rotateZ) model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 0.0f, 1.0f));
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderID, "ka"), ambientColor.r, ambientColor.g, ambientColor.b);
        glUniform3f(glGetUniformLocation(shaderID, "kd"), diffuseColor.r, diffuseColor.g, diffuseColor.b);
        glUniform3f(glGetUniformLocation(shaderID, "ks"), specularColor.r, specularColor.g, specularColor.b);
        glUniform3f(glGetUniformLocation(shaderID, "ke"), emissiveColor.r, emissiveColor.g, emissiveColor.b);
        glUniform1f(glGetUniformLocation(shaderID, "q"), geometry.shininess);
        glUniform3f(glGetUniformLocation(shaderID, "lightPos"), 0.0f, 2.0f, 0.0f);
        glUniform3f(glGetUniformLocation(shaderID, "lightColor"), 1.3f, 1.3f, 1.3f);
        glUniform3f(glGetUniformLocation(shaderID, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glActiveTexture(GL_TEXTURE0);
        if (geometry.textureID > 0) glBindTexture(GL_TEXTURE_2D, geometry.textureID);
        glUniform1i(glGetUniformLocation(shaderID, "colorBuffer"), 0); 
        glBindVertexArray(geometry.VAO);
        glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1, &geometry.VAO);
    glfwTerminate();
    return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
				if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
				if (key == GLFW_KEY_X) { rotateX = !rotateX; rotateY = false; rotateZ = false; }
				if (key == GLFW_KEY_Y) { rotateX = false; rotateY = !rotateY; rotateZ = false; }
				if (key == GLFW_KEY_Z) { rotateX = false; rotateY = false; rotateZ = !rotateZ; }
		}
}

void continous_key_press(GLFWwindow* window, Camera& camera, float currentTime)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("FORWARD", currentTime);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("BACKWARD", currentTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("LEFT", currentTime);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("RIGHT", currentTime);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static float sensitivity = 0.1f;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.Yaw += xoffset;
    camera.Pitch += yoffset;

    if (camera.Pitch > 89.0f)
    {
        camera.Pitch = 89.0f;
    }
    if (camera.Pitch < -89.0f)
    {
        camera.Pitch = -89.0f;
    }

    camera.updateCameraVectors();
}

int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int loadTexture(const string& path)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cerr << "Failed to load texture: " << path << endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

bool loadObject(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals)
{
	std::ifstream file(path);
	if (!file)
	{
			std::cerr << "Failed to open file: " << path << std::endl;
			return false;
	}
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
	std::string line;
	while (std::getline(file, line))
	{
			std::istringstream iss(line);
			std::string type;
			iss >> type;

			if (type == "v") 
			{
					glm::vec3 vertex;
					iss >> vertex.x >> vertex.y >> vertex.z;
					temp_vertices.push_back(vertex);
			}
			else if (type == "vt") 
			{
					glm::vec2 uv;
					iss >> uv.x >> uv.y;
					temp_uvs.push_back(uv);
			}
			else if (type == "vn") 
			{
					glm::vec3 normal;
					iss >> normal.x >> normal.y >> normal.z;
					temp_normals.push_back(normal);
			}
			else if (type == "f")
			{
					unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
					char slash;

					for (int i = 0; i < 3; ++i)
					{
							iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];
							vertexIndices.push_back(vertexIndex[i]);
							uvIndices.push_back(uvIndex[i]);
							normalIndices.push_back(normalIndex[i]);
					}
			}
			else if (type == "mtllib")
			{
					iss >> mtlFilePath;
			}
	}
	for (unsigned int i = 0; i < vertexIndices.size(); ++i)
	{
			unsigned int vertexIndex = vertexIndices[i];
			unsigned int uvIndex = uvIndices[i];
			unsigned int normalIndex = normalIndices[i];
			glm::vec3 vertex = temp_vertices[vertexIndex - 1];
			glm::vec2 uv = temp_uvs[uvIndex - 1];
			glm::vec3 normal = temp_normals[normalIndex - 1];
			out_vertices.push_back(vertex);
			out_uvs.push_back(uv);
			out_normals.push_back(normal);
	}
	file.close();
	return true;
}

Geometry setupGeometry(const char* filepath)
{
    std::vector<GLfloat> vertices;
    std::vector<glm::vec3> vert;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    loadObject(filepath, vert, uvs, normals);
    vertices.reserve(vert.size() * 8); 
    for (size_t i = 0; i < vert.size(); ++i)
    {
      vertices.insert(vertices.end(), {
				vert[i].x, vert[i].y, vert[i].z,
				uvs[i].x, uvs[i].y,
				normals[i].x, normals[i].y, normals[i].z
				}
			);
    }
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    Geometry geom;
    geom.VAO = VAO;
    geom.vertexCount = vertices.size() / 6;
    string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
    string mtlPath = basePath + "/" + mtlFilePath;
    Material mat = loadMTL(mtlPath);
    geom.ka = mat.ka;
    geom.kd = mat.kd;
    geom.ks = mat.ks;
    geom.ke = mat.ke;
    geom.shininess = mat.shininess;
    geom.textureFilePath = mat.texturePath;
    if (!mat.texturePath.empty())
    {
        string fullTexturePath = basePath + "/" + mat.texturePath;
        geom.textureID = loadTexture(fullTexturePath);
        geom.textureFilePath = fullTexturePath;
    }
    return geom;
}

Material loadMTL(const string& path)
{
	Material mat;
	ifstream mtlFile(path);
	if (!mtlFile)
	{
			cerr << "Failed to open MTL file: " << path << endl;
			return mat;
	}

	string line;
	while (getline(mtlFile, line))
	{
			istringstream iss(line);
			string keyword;
			iss >> keyword;

			if (keyword == "map_Kd")
			{
					iss >> mat.texturePath;
			}
			else if (keyword == "Ka")
			{
					iss >> mat.ka.r >> mat.ka.g >> mat.ka.b;
			}
			else if (keyword == "Kd")
			{
					iss >> mat.kd.r >> mat.kd.g >> mat.kd.b;
			}
			else if (keyword == "Ks")
			{
					iss >> mat.ks.r >> mat.ks.g >> mat.ks.b;
			}
			else if (keyword == "Ke")
			{
					iss >> mat.ke.r >> mat.ke.g >> mat.ke.b;
			}
			else if (keyword == "Ns")
			{
					iss >> mat.shininess;
			}
	}
	std::cout << mat.texturePath << std::endl;
	return mat;
}