/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// LoadSimpleObj
#include "../include/LoadSimpleObj.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 600, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"out vec4 finalColor;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = projection * view * model * vec4(position, 1.0);\n"
"    finalColor = vec4(color, 1.0);\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

// Variaveis globais para coordenada atual do cubo
float posX = 0.0f;
float posZ = 0.0f;
float posY = 0.0f;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 10.0f); // Posição inicial da câmera
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Direção para onde a câmera está olhando
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f); // Vetor para cima

int nVertices1 = 0, nVertices2 = 0;
GLuint VAO1, VAO2;

// Seleção de objeto atual
int selectedObject = 1; // 1 para o primeiro, 2 para o segundo

// Transformações separadas (posição no eixo X como exemplo)
float pos1X = -2.0f, pos1Y = 0.0f, pos1Z = 0.0f;
float pos2X =  2.0f, pos2Y = 0.0f, pos2Z = 0.0f;
bool rotate1X = false, rotate1Y = false, rotate1Z = false;
bool rotate2X = false, rotate2Y = false, rotate2Z = false;


// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();

    // Versão do OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criação da janela
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Lucas Weber!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Callback de teclado
    glfwSetKeyCallback(window, key_callback);

    // Inicializa GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Informações da GPU
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported: " << glGetString(GL_VERSION) << std::endl;

    // Configuração da viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilação dos shaders e geometria
    GLuint shaderID = setupShader();

	// Carrega os dois modelos Suzanne
    VAO1 = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVertices1);
	VAO2 = loadSimpleOBJ("../assets/Modelos3D/Cube.obj", nVertices2);

    glUseProgram(shaderID);

    // Localizações dos uniforms
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    glEnable(GL_DEPTH_TEST);

    // Loop principal
    while (!glfwWindowShouldClose(window))
    {
        // Eventos
        glfwPollEvents();

        // Limpa buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Tempo para animação
        float angle = (GLfloat)glfwGetTime();

        
        // MATRIZ VIEW (câmera)
        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );

        // MATRIZ PROJECTION (perspectiva)
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)width / height,
            0.1f,
            100.0f
        );

		// Configurações comuns
        glLineWidth(10);
        glPointSize(20);

		// === Primeira Suzanne (VAO1) ===
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(pos1X, pos1Y, pos1Z));
        if (rotate1X) model1 = glm::rotate(model1, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        else if (rotate1Y) model1 = glm::rotate(model1, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        else if (rotate1Z) model1 = glm::rotate(model1, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, nVertices1);

        // === Segunda Suzanne (VAO2) ===
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(pos2X, pos2Y, pos2Z));
        if (rotate2X) model2 = glm::rotate(model2, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        else if (rotate2Y) model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        else if (rotate2Z) model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, nVertices2);

        glBindVertexArray(0);

		/* Removido para desenho de dois VAOs
        // Desenho
        glLineWidth(10);
        glPointSize(20);
        glDrawArrays(GL_TRIANGLES, 0, nVertices);  // Preenchido
        // glDrawArrays(GL_POINTS, 0, nVertices);     // Pontos
        glBindVertexArray(0);
		*/

        // Troca de buffers
        glfwSwapBuffers(window);
    }

    // Limpeza
    glDeleteVertexArrays(1, &VAO1);
    glDeleteVertexArrays(1, &VAO2);
    glfwTerminate();
    return 0;
}


// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const float moveStep = 0.1f;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, true);

        // Seleciona objeto 1 ou 2
        if (key == GLFW_KEY_1) selectedObject = 1;
        if (key == GLFW_KEY_2) selectedObject = 2;

        // Movimentação do objeto selecionado
        if (key == GLFW_KEY_LEFT) {
            if (selectedObject == 1) pos1X -= moveStep;
            else pos2X -= moveStep;
        }
        if (key == GLFW_KEY_RIGHT) {
            if (selectedObject == 1) pos1X += moveStep;
            else pos2X += moveStep;
        }
        if (key == GLFW_KEY_UP) {
            if (selectedObject == 1) pos1Y += moveStep;
            else pos2Y += moveStep;
        }
        if (key == GLFW_KEY_DOWN) {
            if (selectedObject == 1) pos1Y -= moveStep;
            else pos2Y -= moveStep;
        }

        // Zoom individual (movimento no eixo Z do objeto)
        if (key == GLFW_KEY_I) {
            if (selectedObject == 1) pos1Z += moveStep;
            else pos2Z += moveStep;
        }
        if (key == GLFW_KEY_J) {
            if (selectedObject == 1) pos1Z -= moveStep;
            else pos2Z -= moveStep;
        }

        // Rotação do objeto selecionado
        if (key == GLFW_KEY_X) {
            if (selectedObject == 1) {
                rotate1X = true; rotate1Y = false; rotate1Z = false;
            } else {
                rotate2X = true; rotate2Y = false; rotate2Z = false;
            }
        }
        if (key == GLFW_KEY_Y) {
            if (selectedObject == 1) {
                rotate1X = false; rotate1Y = true; rotate1Z = false;
            } else {
                rotate2X = false; rotate2Y = true; rotate2Z = false;
            }
        }
        if (key == GLFW_KEY_Z) {
            if (selectedObject == 1) {
                rotate1X = false; rotate1Y = false; rotate1Z = true;
            } else {
                rotate2X = false; rotate2Y = false; rotate2Z = true;
            }
        }

        // Movimento da câmera global (continua funcionando normalmente)
        if (key == GLFW_KEY_W)
            cameraPos -= moveStep * cameraUp;
        if (key == GLFW_KEY_S)
            cameraPos += moveStep * cameraUp;
        if (key == GLFW_KEY_A)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * moveStep;
        if (key == GLFW_KEY_D)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * moveStep;
    }
}

// Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}