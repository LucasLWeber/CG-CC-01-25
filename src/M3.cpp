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
 #include "../include/LoadSimpleObj.h"

 #define STB_IMAGE_IMPLEMENTATION
 #include <stb_image.h>
 
 // Protótipo da função de callback de teclado
 void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
 
 // Protótipos das funções
 int setupShader();
 int setupGeometry();
 GLuint loadTexture(std::string filePath, int &width, int &height);
 
 // Dimensões da janela (pode ser alterado em tempo de execução)
 const GLuint WIDTH = 600, HEIGHT = 600;
 
 // Código fonte do Vertex Shader (em GLSL): ainda hardcoded
 const GLchar *vertexShaderSource = R"(
		#version 400
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec2 texc;

    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;

    out vec2 texCoord;

    void main()
    {
			gl_Position = projection * view * model * vec4(position, 1.0);
			texCoord = texc;
		}
)";
	
 
 //Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
 const GLchar *fragmentShaderSource = R"(
	#version 400
	in vec2 texCoord;
	uniform sampler2D texBuff;
	out vec4 color;
	void main()
	{
		color = texture(texBuff,texCoord);
	})";
 
 // Variaveis globais para coordenada atual do cubo
 float posX = 0.0f;
 float posZ = 0.0f;
 float posY = 0.0f;
 
 glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 15.0f); // Posição inicial da câmera
 glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Direção para onde a câmera está olhando
 glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f); // Vetor para cima
 
 bool rotateX=false, rotateY=false, rotateZ=false;
 GLuint VAO;
 int nVertices = 0;
 
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

		 // Caminho relativo nao está funcionando
		 VAO = loadSimpleOBJ("D:/ComputacaoGrafica/RepoAulas/repo-prof/CGCCHibrido/assets/Modelos3D/Cube.obj", nVertices);
		 int texWidth, texHeight;
		 GLuint textureID = loadTexture("D:/ComputacaoGrafica/RepoAulas/repo-prof/CGCCHibrido/assets/Modelos3D/Cube.png", texWidth, texHeight);
	

		 glUseProgram(shaderID);
		 glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
		 glActiveTexture(GL_TEXTURE0);
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

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(posX, posY, posZ));
        if (rotateX) model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        else if (rotateY) model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        else if (rotateZ) model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

				// ✅ ATIVA A TEXTURA e ENVIA PARA O SHADER
			  glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureID);
				glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0); 

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, nVertices);
        glBindVertexArray(0);

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
	   // std::cout << "Key: " << key << " Action: " << action << std::endl;
		 const float cameraSpeed = 0.1f;
 
		 if (action == GLFW_PRESS || action == GLFW_REPEAT)
		 {
				 if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
 
				 // Rotação do cubo (opcional)
				 if (key == GLFW_KEY_X) { rotateX = !rotateX; rotateY = false; rotateZ = false; }
				 if (key == GLFW_KEY_Y) { rotateX = false; rotateY = !rotateY; rotateZ = false; }
				 if (key == GLFW_KEY_Z) { rotateX = false; rotateY = false; rotateZ = !rotateZ; }
 
				 // Movimento da câmera
				 if (key == GLFW_KEY_W) cameraPos -= cameraSpeed * cameraUp;
				 if (key == GLFW_KEY_S) cameraPos += cameraSpeed * cameraUp;
				 if (key == GLFW_KEY_A) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
				 if (key == GLFW_KEY_D) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
				 if (key == GLFW_KEY_I) cameraPos += cameraSpeed * cameraFront;
				 if (key == GLFW_KEY_J) cameraPos -= cameraSpeed * cameraFront;
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
 
GLuint loadTexture(string filePath, int &width, int &height)
{
	GLuint texID; // id da textura a ser carregada

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Ajuste dos parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento da imagem usando a função stbi_load da biblioteca stb_image
	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // assume que é 4 canais png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture " << filePath << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}