#include <iostream>
#include <string>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <sstream>
#include <fstream>

using namespace std;

const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texc;
uniform mat4 projection;
uniform mat4 model;
out vec2 texCoord;
out vec3 vNormal;
out vec4 fragPos; 
out vec4 vColor;
void main()
{
	gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
	fragPos = model * vec4(position.x, position.y, position.z, 1.0);
	texCoord = texc;
	vNormal = normal;
	vColor = vec4(color,1.0);
}
)";

const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoord;
uniform sampler2D texBuff;
uniform vec3 lightPos;
uniform vec3 fillLightPos;    
uniform vec3 fillLightColor;
uniform int keyLightOn;
uniform int fillLightOn;
uniform vec3 backLightPos;
uniform vec3 backLightColor;
uniform int backLightOn;
uniform vec3 camPos;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float q;
out vec4 color;
in vec4 fragPos;
in vec3 vNormal;
in vec4 vColor;
void main()
{
	vec3 lightColor = vec3(1.0,1.0,1.0);
	vec4 objectColor = vColor;
	vec3 ambient = ka * lightColor;
	vec3 N = normalize(vNormal);
	vec3 diffuse = vec3(0.0);
	vec3 diffuseFill = vec3(0.0);
	vec3 specular = vec3(0.0);
	vec3 diffuseBack = vec3(0.0);
	if (keyLightOn == 1) {
			vec3 L = normalize(lightPos - vec3(fragPos));
			float diff = max(dot(N, L), 0.0);
			diffuse = kd * diff * lightColor;
			vec3 R = normalize(reflect(-L, N));
			vec3 V = normalize(camPos - vec3(fragPos));
			float spec = max(dot(R, V), 0.0);
			spec = pow(spec, q);
			specular = ks * spec * lightColor;
	}
	if (fillLightOn == 1) {
			vec3 Lfill = normalize(fillLightPos - vec3(fragPos));
			float diffFill = max(dot(N, Lfill), 0.0);
			float distFill = length(fillLightPos - vec3(fragPos));
			float attenuationFill = 1.0 / (distFill * distFill);
			diffuseFill = kd * diffFill * fillLightColor * attenuationFill;
	}
	if (backLightOn == 1) {
		vec3 Lback = normalize(backLightPos - vec3(fragPos));
		float diffBack = max(dot(N, Lback), 0.0);
		float distBack = length(backLightPos - vec3(fragPos));
		float attenuationBack = 1.0 / (distBack * distBack);
		diffuseBack = kd * diffBack * backLightColor * attenuationBack;
	}
	vec3 result = ambient * vec3(objectColor) + diffuse * vec3(objectColor) + diffuseFill + specular + diffuseBack;
	color = vec4(result,1.0);
}
)";

struct Geometry {
	GLuint VAO;
	GLuint nVertices;
	GLuint textureID = 0;
	string textureFilePath;
};

int setupShader();
Geometry setupGeometry(const char* filepath);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
bool loadObject(const char* path, vector<glm::vec3>& out_vertices, vector<glm::vec2>& out_uvs, vector<glm::vec3>& out_normals);
void drawGeometry(
	GLuint shaderID, 
	GLuint VAO, 
	glm::vec3 position, 
	glm::vec3 dimensions, 
	float angle, 
	GLuint nVertices, 
	glm::vec3 color = glm::vec3(1.0,0.0,0.0), 
	glm::vec3 axis  = (glm::vec3(0.0, 0.0, 1.0)));

const GLuint WIDTH = 800, HEIGHT = 600;
glm::vec3 lightPos;
bool keyLightOn = true;
bool fillLightOn = true;
bool backLightOn = true;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.0f);

int main (){
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vivencial 2", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
			std::cout << "Failed to initialize GLAD" << std::endl;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();
	Geometry geom = setupGeometry("D:/ComputacaoGrafica/RepoAulas/repo-prof/CGCCHibrido/assets/Modelos3D/SuzanneSubdiv1.obj");

	lightPos = glm::vec3(0.6, 1.2, -0.5);

	float ka = 0.1, kd =1.0, ks = 0.5, q = 10.0;

	lightPos 									= glm::vec3(0.6, 1.2, -0.5);
	glm::vec3 fillLightPos 		= glm::vec3(-1.0f, -1.0f, -1.0f);
	glm::vec3 fillLightColor 	= glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 backLightPos 		= glm::vec3(-1.0f, 1.0f, -1.0f);
	glm::vec3 backLightColor 	= glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 camPos 					= cameraPos;

	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
	glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
	glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
	glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
	glUniform1f(glGetUniformLocation(shaderID, "q"), q);
	glUniform3f(glGetUniformLocation(shaderID, "lightPos"), lightPos.x,lightPos.y,lightPos.z);
	glUniform3f(glGetUniformLocation(shaderID, "fillLightPos"), fillLightPos.x,fillLightPos.y,fillLightPos.z);
	glUniform3f(glGetUniformLocation(shaderID, "fillLightColor"), fillLightColor.x,fillLightColor.y,fillLightColor.z);
	glUniform3f(glGetUniformLocation(shaderID, "backLightPos"), backLightPos.x,backLightPos.y,backLightPos.z);
	glUniform3f(glGetUniformLocation(shaderID, "backLightColor"), backLightColor.x,backLightColor.y,backLightColor.z);
	glUniform3f(glGetUniformLocation(shaderID, "camPos"), camPos.x,camPos.y,camPos.z);
	glActiveTexture(GL_TEXTURE0);

	glm::mat4 projection = glm::ortho(-1.0, 1.0, -1.0, 1.0, -3.0, 3.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform3f(glGetUniformLocation(shaderID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform1i(glGetUniformLocation(shaderID, "keyLightOn"), keyLightOn);
		glUniform1i(glGetUniformLocation(shaderID, "fillLightOn"), fillLightOn);
		glUniform1i(glGetUniformLocation(shaderID, "backLightOn"), backLightOn);

		glBindVertexArray(geom.VAO);
		drawGeometry(
			shaderID, 
			geom.VAO, 
			glm::vec3(0, 0, 0), 
			glm::vec3(0.3, 0.3, 0.3), 
			0.0, 
			geom.nVertices
		);
		glBindVertexArray(0); 

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &geom.VAO);
	glfwTerminate();
	return 0;
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
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
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
      vertices.insert(vertices.end(),
        {
          vert[i].x, vert[i].y, vert[i].z,
          uvs[i].x, uvs[i].y,
          normals[i].x, normals[i].y, normals[i].z
          }
				);
    }
    GLuint VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Geometry geom;
    geom.VAO = VAO;
    geom.nVertices = vertices.size() / 8;		
    return geom;
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

void drawGeometry(GLuint shaderID, GLuint VAO, glm::vec3 position, glm::vec3 dimensions, float angle, GLuint nVertices, glm::vec3 color, glm::vec3 axis)
{
	glm::mat4 model = glm::mat4(1.0f); 
	model = translate(model, position);
	model = rotate(model, glm::radians(angle), axis);
	model = scale(model, dimensions);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)				lightPos.x -= 0.2;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) 			lightPos.x += 0.2;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) 			lightPos.y -= 0.2;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)					lightPos.y += 0.2;
	if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS)	lightPos.z -= 0.2;
	if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS)		lightPos.z += 0.2;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)					keyLightOn = !keyLightOn;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)					fillLightOn = !fillLightOn;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)					backLightOn = !backLightOn;
}