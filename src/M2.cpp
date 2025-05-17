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


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

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

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f); // Posição inicial da câmera
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Direção para onde a câmera está olhando
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f); // Vetor para cima


bool rotateX=false, rotateY=false, rotateZ=false;

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
    GLuint VAO = setupGeometry();

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

        // MATRIZ MODEL
        glm::mat4 model = glm::mat4(1.0f);
        if (rotateX)
            model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        else if (rotateY)
            model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        else if (rotateZ)
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

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

        // Enviando matrizes para o shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Desenho
        glLineWidth(10);
        glPointSize(20);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);  // Preenchido
        glDrawArrays(GL_POINTS, 0, 36);     // Pontos
        glBindVertexArray(0);

        // Troca de buffers
        glfwSwapBuffers(window);
    }

    // Limpeza
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}


// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const float cameraSpeed = 0.1f;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, true);

        // Rotação do cubo (opcional)
        if (key == GLFW_KEY_X) { rotateX = true; rotateY = false; rotateZ = false; }
        if (key == GLFW_KEY_Y) { rotateX = false; rotateY = true; rotateZ = false; }
        if (key == GLFW_KEY_Z) { rotateX = false; rotateY = false; rotateZ = true; }

        // Movimento da câmera
        if (key == GLFW_KEY_W)
            cameraPos += cameraSpeed * cameraFront;
        if (key == GLFW_KEY_S)
            cameraPos -= cameraSpeed * cameraFront;
        if (key == GLFW_KEY_A)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_D)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_I)
            cameraPos += cameraSpeed * cameraUp;
        if (key == GLFW_KEY_J)
            cameraPos -= cameraSpeed * cameraUp;
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

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {
		// Frente (vermelho)
		-0.5, -0.5,  0.5,  1.0, 0.0, 0.0,
		 0.5, -0.5,  0.5,  1.0, 0.0, 0.0,
		 0.5,  0.5,  0.5,  1.0, 0.0, 0.0,

		-0.5, -0.5,  0.5,  1.0, 0.0, 0.0,
		 0.5,  0.5,  0.5,  1.0, 0.0, 0.0,
		-0.5,  0.5,  0.5,  1.0, 0.0, 0.0,

		// Trás (verde)
		-0.5, -0.5, -0.5,  0.0, 1.0, 0.0,
		 0.5,  0.5, -0.5,  0.0, 1.0, 0.0,
		 0.5, -0.5, -0.5,  0.0, 1.0, 0.0,

		-0.5, -0.5, -0.5,  0.0, 1.0, 0.0,
		-0.5,  0.5, -0.5,  0.0, 1.0, 0.0,
		 0.5,  0.5, -0.5,  0.0, 1.0, 0.0,

		// Esquerda (azul)
		-0.5, -0.5, -0.5,  0.0, 0.0, 1.0,
		-0.5, -0.5,  0.5,  0.0, 0.0, 1.0,
		-0.5,  0.5,  0.5,  0.0, 0.0, 1.0,

		-0.5, -0.5, -0.5,  0.0, 0.0, 1.0,
		-0.5,  0.5,  0.5,  0.0, 0.0, 1.0,
		-0.5,  0.5, -0.5,  0.0, 0.0, 1.0,

		// Direita (amarelo)
		 0.5, -0.5, -0.5,  1.0, 1.0, 0.0,
		 0.5,  0.5,  0.5,  1.0, 1.0, 0.0,
		 0.5, -0.5,  0.5,  1.0, 1.0, 0.0,

		 0.5, -0.5, -0.5,  1.0, 1.0, 0.0,
		 0.5,  0.5, -0.5,  1.0, 1.0, 0.0,
		 0.5,  0.5,  0.5,  1.0, 1.0, 0.0,

		// Superior (ciano)
		-0.5,  0.5, -0.5,  0.0, 1.0, 1.0,
		-0.5,  0.5,  0.5,  0.0, 1.0, 1.0,
		 0.5,  0.5,  0.5,  0.0, 1.0, 1.0,

		-0.5,  0.5, -0.5,  0.0, 1.0, 1.0,
		 0.5,  0.5,  0.5,  0.0, 1.0, 1.0,
		 0.5,  0.5, -0.5,  0.0, 1.0, 1.0,

		// Inferior (magenta)
		-0.5, -0.5, -0.5,  1.0, 0.0, 1.0,
		 0.5, -0.5,  0.5,  1.0, 0.0, 1.0,
		-0.5, -0.5,  0.5,  1.0, 0.0, 1.0,

		-0.5, -0.5, -0.5,  1.0, 0.0, 1.0,
		 0.5, -0.5, -0.5,  1.0, 0.0, 1.0,
		 0.5, -0.5,  0.5,  1.0, 0.0, 1.0
	};
	
		
	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

