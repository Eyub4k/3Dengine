// Eyub Celebioglu
#include <glad.h>
#include <glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"

// struct d'un sommet
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

// vecteur pour stocker les données du modele
std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

// fnc pour charger un fichier .obj
bool loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec2> temp_texcoords;

    std::string line;
    int faceCount = 0; // compteur de face
    while (std::getline(file, line)) {
        std::istringstream s(line);
        std::string type;
        s >> type;

        if (type == "v") {
            glm::vec3 position;
            s >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        } else if (type == "vt") {
            glm::vec2 texcoord;
            s >> texcoord.x >> texcoord.y;
            temp_texcoords.push_back(texcoord);
        } else if (type == "vn") {
            glm::vec3 normal;
            s >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        } else if (type == "f") {
            unsigned int vIndex[3], tIndex[3], nIndex[3];
            char slash;
            for (int i = 0; i < 3; i++) {
                s >> vIndex[i] >> slash >> tIndex[i] >> slash >> nIndex[i];
                vIndex[i]--; tIndex[i]--; nIndex[i]--;
            }
            for (int i = 0; i < 3; i++) {
                Vertex vertex;
                vertex.Position = temp_positions[vIndex[i]];
                vertex.TexCoords = temp_texcoords[tIndex[i]];
                vertex.Normal = temp_normals[nIndex[i]];
                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
            faceCount++;
        }
    }

    file.close();

    std::cout << "Chargement du modèle terminé !" << std::endl;
    std::cout << "Nombre de sommets : " << vertices.size() << std::endl;
    std::cout << "Nombre de faces : " << faceCount << std::endl;

    return true;
}

// OpenGL buffers
GLuint modelVAO, modelVBO, modelEBO;

// config du mesh chargee 
void setupMesh() {
    glGenVertexArrays(1, &modelVAO);
    glGenBuffers(1, &modelVBO);
    glGenBuffers(1, &modelEBO);

    glBindVertexArray(modelVAO);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glBindVertexArray(0);

    // on affiche les u,v (doivent etre comprise entre 0 et 1 sinon pb)
    std::cout << "UV Coordonnées : " << vertices[0].TexCoords.x << ", " << vertices[0].TexCoords.y << std::endl;

}

// gestion des inputs
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn), ypos = static_cast<float>(yposIn);
    if (firstMouse) { 
        lastX = xpos; 
        lastY = ypos; 
        firstMouse = false; 
    }
    float xoffset = xpos - lastX, 
    float yoffset = lastY - ypos;
    lastX = xpos; 
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(3, deltaTime);
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        std::cout << "Texture loaded: " << path 
                  << " | Width: " << width 
                  << ", Height: " << height 
                  << ", Channels: " << nrChannels << std::endl;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}

// fnc pour calculer le rayon depuis la souris
glm::vec3 screenToWorld(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view) {
    glm::vec4 clipCoords(
        (2.0f * mouseX) / 800.0f - 1.0f,
        1.0f - (2.0f * mouseY) / 600.0f,
        -1.0f,
        1.0f
    );

    glm::mat4 invertedProjection = glm::inverse(projection);
    glm::mat4 invertedView = glm::inverse(view);
    glm::vec4 eyeCoords = invertedProjection * clipCoords;
    eyeCoords.z = -1.0f;  // on pointe dans la direction negatives
    eyeCoords.w = 0.0f;

    glm::vec4 worldCoords = invertedView * eyeCoords;
    glm::vec3 direction = glm::normalize(glm::vec3(worldCoords));

    return direction; // rayon dans l'espace 3D
}

bool intersectRayWithObject(const glm::vec3& rayDirection) {
    return true;
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Eyub Engine", nullptr, nullptr);

    if (!window) { 
        glfwTerminate(); 
        return -1; 
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); });
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    Shader shader("C:/Users/eyubc/OneDrive/Bureau/3Dengine/shaders/vertex_shader.glsl", "C:/Users/eyubc/OneDrive/Bureau/3Dengine/shaders/fragment_shader.glsl");

    if (!loadOBJ("C:/Users/eyubc/OneDrive/Bureau/3Dengine/texture/exemple2.obj")) return -1;
    setupMesh();
    
    GLuint texture = loadTexture("C:/Users/eyubc/OneDrive/Bureau/3Dengine/texture/tex2.jpeg");
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.setInt("ourTexture", 0);


    // Variables de lumière
    glm::vec3 lightPos(1.0f, 1.0f, 1.0f);  // Position de la lumiere
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // Couleur de la lumiere blanc
    glm::vec3 viewPos = camera.Position; // Position de la caméra

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        shader.use();

        // envoie les variables de lumiere au shader
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("viewPos", viewPos);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.01f)); // reduit la taille de l'objet 

        shader.setMat4("model", model);

        glBindVertexArray(modelVAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &modelVAO);
    glDeleteBuffers(1, &modelVBO);
    glDeleteBuffers(1, &modelEBO);
    glfwTerminate();
    return 0;
}
