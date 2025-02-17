#include <glad.h>    // Charge les fonctions OpenGL
#include <glfw3.h>   // Interface de gestion de fenêtres et événements
#include <iostream>   
#include <string>    
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;

// Fonction de gestion du redimensionnement de la fenêtre
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Cette fonction permet de mettre à jour la vue d'OpenGL lorsqu'on redimensionne la fenêtre
    glViewport(0, 0, width, height);
}

// Code source du Vertex Shader (gestion des sommets)
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"  // Définit l'emplacement du sommet dans l'input (location = 0)
    "uniform mat4 model;\n"  // Matrice du modèle (transformation)
    "uniform mat4 view;\n"   // Matrice de la vue (caméra)
    "uniform mat4 projection;\n"  // Matrice de projection (perspective)
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"  // Applique les transformations
    "}\0"; 

// Code source du Fragment Shader (gestion des couleurs des pixels)
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"  // Déclare la couleur de sortie du fragment
    "void main() {\n"
    "   FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"  // Ici on définit une couleur jaune (RGB = 1.0, 1.0, 0.0, 1.0 pour du jaune)
    "}\0";  

int main() {
    // Initialisation de GLFW (gestion de la fenêtre et des événements)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // Définit la version majeure d'OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // Définit la version mineure d'OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Utilise le profil core d'OpenGL

    // Création de la fenêtre GLFW
    GLFWwindow* window = glfwCreateWindow(800, 600, "Eyub Engine", NULL, NULL);  // Taille (800x600)
    if (window == NULL) {  
        cout << "Échec de création de la fenêtre GLFW" << endl;
        glfwTerminate();  
        return -1;
    }
    glfwMakeContextCurrent(window);  // Faire de cette fenêtre la fenêtre contextuelle active
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // Appele la fonction pour gérer les changements de taille de fenêtre

    // Initialisation de GLAD (gestion des extensions OpenGL)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {  // Charge toutes les fonctions OpenGL avec GLAD
        cout << "Échec d'initialisation de GLAD" << endl;
        return -1;
    }

    // Définir les sommets pour trois triangles
    float vertices[] = {
        // Triangle 1 (Haut)
        -0.5f,  0.5f, 0.0f,  // Haut gauche
         0.5f,  0.5f, 0.0f,  // Haut droit
         0.0f,  1.0f, 0.0f,  // Haut central (point du sommet)
    
        // Triangle 2 (Bas gauche)
        -0.5f, 0.5f, 0.0f,  // Bas gauche
        -1.0f,  0.0f, 0.0f,  // Bas gauche extérieur
        -0.0f,  0.0f, 0.0f,  // Bas milieu gauche
    
        // Triangle 3 (Bas droit)
        0.5f, 0.5f, 0.0f,   // Bas droit
        1.0f,  0.0f, 0.0f,   // Bas droit extérieur
        0.0f,  0.0f, 0.0f    // Bas milieu droit
    };

    // Création et configuration des VAO (Vertex Array Object) et VBO (Vertex Buffer Object)
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);  // Crée un VAO (un objet qui stocke les paramètres de configuration)
    glGenBuffers(1, &VBO);  // Crée un VBO pour stocker les données de vertex dans la mémoire GPU

    glBindVertexArray(VAO);  // Active le VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Lie le VBO à GL_ARRAY_BUFFER pour qu'on puisse y stocker les données des sommets
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // Copie les données des sommets dans le VBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);  // Déclare comment accéder aux données de vertex
    glEnableVertexAttribArray(0);  // Active l'attribut de vertex pour le VAO
    glBindVertexArray(0);  // Désactive le VAO pour ne pas le modifier par accident

    // Création et compilation des shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);  // Crée le shader de vertex
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);  // Charge le code source du vertex shader
    glCompileShader(vertexShader);  // Compile le vertex shader

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);  // Crée le shader de fragment
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);  // Charge le code source du fragment shader
    glCompileShader(fragmentShader);  // Compile le fragment shader

    // Création du programme shader
    unsigned int shaderProgram = glCreateProgram();  // Crée un programme pour lier les shaders
    glAttachShader(shaderProgram, vertexShader);  // Attache le shader de vertex au programme
    glAttachShader(shaderProgram, fragmentShader);  // Attache le shader de fragment au programme
    glLinkProgram(shaderProgram);  // Lie les shaders pour créer un programme complet
    glDeleteShader(vertexShader);  // Supprime le shader de vertex après l'avoir lié au programme
    glDeleteShader(fragmentShader);  // Supprime le shader de fragment après l'avoir lié au programme

    // Matrices de transformation 3D
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);  // Projection perspective (pas trop grande ici)
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // Position de la caméra
    glm::mat4 model = glm::mat4(1.0f);  // Matrice d'identité (pas de transformation)

    // Boucle principale de rendu
    while (!glfwWindowShouldClose(window)) { 
        glClear(GL_COLOR_BUFFER_BIT);  // Efface l'écran avec la couleur de fond par défaut (noir)

        glUseProgram(shaderProgram);  // Utilise le programme shader pour dessiner

        model = glm::rotate(model, glm::radians(0.3f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation autour de l'axe Y

        // Envoyer les matrices au shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model"); // Obtenir l'emplacement de la variable  model dans le shader (pour la transformation de l'objet)
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view"); // Obtenir l'emplacement de la variable view dans le shader (pour la caméra)
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection"); // Obtenir l'emplacement de la variable projection dans le shader (pour la projection 3D)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); //Envoye la matrice model (transformation de l'objet) au shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); // Envoyer la matrice view (vue de la caméra) au shader
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection)); // Envoyer la matrice projection (projection 3D) au shader

        glBindVertexArray(VAO);  // Lie le VAO pour qu'il soit utilisé dans le rendu
        glDrawArrays(GL_TRIANGLES, 0, 9);  // Dessine les triangles (9 sommets au total)

        glfwSwapBuffers(window);  // Échange les buffers pour afficher l'image rendue à l'écran
        glfwPollEvents();  // Vérifie les événements (ex : fermer la fenêtre, touches pressées...)
    }

    // Nettoyage des ressources après utilisation
    glDeleteVertexArrays(1, &VAO);  // Supprime le VAO
    glDeleteBuffers(1, &VBO);  // Supprime le VBO
    glDeleteProgram(shaderProgram);  // Supprime le programme shader
    glfwTerminate();  // Termine proprement GLFW

    return 0;  // Termine l'exécution du programme
}
