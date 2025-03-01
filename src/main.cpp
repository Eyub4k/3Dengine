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

// struct pour le sol (plan)
struct Ground {
    GLuint VAO, VBO;
    glm::vec3 position;
    glm::vec3 scale;
    
    Ground() : position(0.0f, -2.0f, 0.0f), scale(20.0f, 1.0f, 20.0f) {
        float vertices[] = {
            // positions          // normales         // coord de texture
            -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,
             0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  20.0f, 0.0f,
             0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  20.0f, 20.0f,
            -0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  20.0f
        };
        
        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        
        GLuint EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        
        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        
        // texture
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        
        glBindVertexArray(0);
    }
    
    void Draw(Shader& shader, GLuint texture) {
        shader.use();
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, scale);
        
        shader.setMat4("model", model);
        
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

struct PhysicsProperties {
    glm::vec3 position;      // position de l'objet
    glm::vec3 velocity;      // vitesse de l'objet
    glm::vec3 acceleration;  // acc de l'objet
    float mass;              // masse de l'objet
    float restitution;       // coef de restitution (rebond) entre 0 et 1
    bool isStatic;           // si l'objet est statique (comme le sol) ou dynamique
    
    PhysicsProperties() : 
        position(0.0f),
        velocity(0.0f),
        acceleration(0.0f, -2.5f, 0.0f), // gravite reduite (au lieu de -9.81) / vous pouvez le changer 
        mass(1.0f),
        restitution(0.8f),   // rebond
        isStatic(false)
    {}
};

// struct pour representer un objet physique
struct PhysicsObject {
    glm::vec3 position;      // position actuelle
    glm::vec3 scale;         // echelle de l'objet
    glm::vec3 minBounds;     // lim min de la boite englobante
    glm::vec3 maxBounds;     // lim max de la boite englobante
    PhysicsProperties physics;
    
    PhysicsObject(const glm::vec3& pos, const glm::vec3& scl) :
        position(pos),
        scale(scl)
    {
        // calcul de la boite englobante en fonction de l'echelle
        minBounds = position - scale * 0.5f;
        maxBounds = position + scale * 0.5f;
        
        // Init des proprietes physiques
        physics.position = position;
    }
    
    // MAJ des lim de la boite englobante
    void updateBounds() {
        position = physics.position; // MAJ la position de rendu
        minBounds = position - scale * 0.5f;
        maxBounds = position + scale * 0.5f;
    }
};

// Detecte la collision entre un objet et le sol
bool checkCollisionWithGround(PhysicsObject& object, const Ground& ground) {
    // verifie si l'objet est sous ou au niveau du sol
    if (object.minBounds.y <= ground.position.y) {
        return true;
    }
    return false;
}

// rep à la collision avec le sol
void resolveGroundCollision(PhysicsObject& object, const Ground& ground) {
    // calcul la profondeur de pene
    float penetration = ground.position.y - object.minBounds.y;
    
    // repositionne l'objet au-dessus du sol
    object.physics.position.y += penetration;
    
    // applique le rebond: inverser la vitesse Y et applique le coef de restitution
    object.physics.velocity.y = -object.physics.velocity.y * object.physics.restitution;
    
    // Si vitesse est trres faible ap rebonda alors on arrete le mouvement pour eviter des rebonds infinis
    if (std::abs(object.physics.velocity.y) < 0.1f) {
        object.physics.velocity.y = 0.0f;
    }
    
    // MAJ les lim
    object.updateBounds();
}

// MAJ la physique pour un objet
void updatePhysics(PhysicsObject& object, float deltaTime, const Ground& ground) {
    if (object.physics.isStatic) return; // objets statiques ne bougent pas
    
    // facteur de ralentissement (0.5 = deux fois plus lent)
    float slowFactor = 0.5f;
    deltaTime *= slowFactor;
    
    // MAJ la vitesse selon l'accel
    object.physics.velocity += object.physics.acceleration * deltaTime;
    
    // lim la vitesse maximale de chute
    const float maxFallSpeed = 5.0f;
    if (object.physics.velocity.y < -maxFallSpeed) {
        object.physics.velocity.y = -maxFallSpeed;
    }
    
    // MAJ la position selon la vitesse
    object.physics.position += object.physics.velocity * deltaTime;
    
    // MAJ les lim de la boite englobante
    object.updateBounds();
    
    // verifie et resoudre les collisions avec le sol
    if (checkCollisionWithGround(object, ground)) {
        resolveGroundCollision(object, ground);
    }
}

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
Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));
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
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; 
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

bool rayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& boxMin, const glm::vec3& boxMax) {
    float tMin = (boxMin.x - rayOrigin.x) / rayDirection.x;
    float tMax = (boxMax.x - rayOrigin.x) / rayDirection.x;

    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (boxMin.y - rayOrigin.y) / rayDirection.y;
    float tyMax = (boxMax.y - rayOrigin.y) / rayDirection.y;

    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if (tMin > tyMax || tyMin > tMax)
        return false;

    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    float tzMin = (boxMin.z - rayOrigin.z) / rayDirection.z;
    float tzMax = (boxMax.z - rayOrigin.z) / rayDirection.z;

    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if (tMin > tzMax || tzMin > tMax)
        return false;

    return true;
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

bool isMoveMode = true; // Si true : mode déplacement, si false : mode curseur
float lastCursorX = 0.0f, lastCursorY = 0.0f;
bool firstCursorSwitch = false;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // detection de la touche U pour alterner entre mode camera et mode curseur
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && !firstCursorSwitch) {
        firstCursorSwitch = true;
        isMoveMode = !isMoveMode;  // switch l'etat du mode
        std::cout << "Changement de mode: " << (isMoveMode ? "Mode Mouvement" : "Mode Curseur") << std::endl;

        if (isMoveMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // mode mouvement, curseur 
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // mode curseur, curseur visible
        }
    }

    // on utilise les touches W, A, S, D
    if (isMoveMode) {
        // deplacement de la caméra avec W, A, S, D
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(0, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(1, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(2, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(3, deltaTime);

        // deplace la caméra en fonction de la souris uniquement si elle est pas au centre
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // deplace la caméra que si la souris a bouge
        if (lastCursorX != 0.0f || lastCursorY != 0.0f) {
            float xoffset = mouseX - lastCursorX;
            float yoffset = lastCursorY - mouseY;  // inverser le Y pour eviter l'inversion verticale
            camera.ProcessMouseMovement(xoffset, yoffset);
        }

        // remettre la souris au centre pour eviter qu'elle ne dépasse de l'ecran
        glfwSetCursorPos(window, 800.0f / 2.0f, 600.0f / 2.0f);  // ajuste la resolution ici si besoin

        lastCursorX = 800.0f / 2.0f;  // centrer la souris
        lastCursorY = 600.0f / 2.0f;
    }

    // mode curseur, on desactive le controle de la souris et on ne fait que de la rotation avec les fleches
    if (!isMoveMode) {
        // rotation de la camera avec les touches fleche
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.ProcessMouseMovement(0.0f, 1.0f);  // haut
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.ProcessMouseMovement(0.0f, -1.0f); // bas
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.ProcessMouseMovement(-1.0f, 0.0f); // gauche
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.ProcessMouseMovement(1.0f, 0.0f);  // droite
    }

    // raycasting avec les objets en mode curseur
    if (!isMoveMode) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            glm::vec3 rayDirection = screenToWorld(static_cast<int>(mouseX), static_cast<int>(mouseY), 
                glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f), camera.GetViewMatrix());

            glm::vec3 boxMin(-0.5f, -0.5f, -0.5f);
            glm::vec3 boxMax(0.5f, 0.5f, 0.5f);

            glm::vec3 rayOrigin = camera.Position;

            if (rayIntersectsAABB(rayOrigin, rayDirection, boxMin, boxMax)) {
                std::cout << "Ray touche l obj" << std::endl;
            }
        }
    }

    // reset de `firstCursorSwitch` quand la touche est lachee
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE) {
        firstCursorSwitch = false;
    }
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
        std::cout << "Texture : " << path 
                  << " | Width: " << width 
                  << ", Height: " << height 
                  << ", Channels: " << nrChannels << std::endl;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Fail : " << path << std::endl;
    }
    stbi_image_free(data);

    return textureID;
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
    
    Shader shader("3Dengine/shaders/vertex_shader.glsl", "3Dengine/shaders/fragment_shader.glsl");

    if (!loadOBJ("3Dengine/texture/exemple.obj")) return -1;
    setupMesh();
    
    GLuint texture = loadTexture("3Dengine/texture/texture_exemple.jpeg");
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture); 
    shader.setInt("ourTexture", 0);

    // creation du sol 
    Ground ground;
    GLuint groundTexture = loadTexture("3Dengine/texture/ground_exemple.jpg");

    // var de lumiere
    glm::vec3 lightPos(1.0f, 1.0f, 1.0f);  // position de la lumiere
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // couleur de la lumiere blanc
    glm::vec3 viewPos = camera.Position; // position de la caméra

    // Init de l'objet physique pour le modele 3D
    PhysicsObject modelObject(glm::vec3(0.0f, 10.0f, 0.0f), // position de depart plus haute (10 au lieu de 5)
                         glm::vec3(0.5f));             // echelle approximativee
    modelObject.physics.restitution = 0.8f; // coef de rebond

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        processInput(window);
        
        // MAJ de la physique
        updatePhysics(modelObject, deltaTime, ground);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shader.use();
        
        // var de la lumiere
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("viewPos", camera.Position);
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        
        // dessiner le sol
        ground.Draw(shader, groundTexture);
        
        // dessiner le modele principal avec sa position MAJ
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, modelObject.position); // utilise la position mise à jour
        model = glm::scale(model, glm::vec3(0.01f));         // echelle d'origine
        shader.setMat4("model", model);
        
        glBindVertexArray(modelVAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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
