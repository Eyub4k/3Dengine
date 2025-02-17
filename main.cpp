#include <glad.h>    // Charge les fonctions OpenGL
#include <glfw3.h>   // Interface de gestion de fenêtres et événements
#include <iostream>   
#include <string>    

using namespace std ;

// Fonction de gestion du redimensionnement de la fenêtre
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Cette fonction permet de mettre à jour la vue d'OpenGL lorsqu'on redimensionne la fenêtre
    glViewport(0, 0, width, height);
}

// Code source du Vertex Shader (gestion des sommets)
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"  // Définit l'emplacement du sommet dans l'input (location = 0)
    "void main() {\n"
    "   gl_Position = vec4(aPos, 1.0);\n"    // La position finale du sommet, convertie en coordonnées homogènes
    "}\0"; 

// Code source du Fragment Shader (gestion des couleurs des pixels)
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"  // Déclare la couleur de sortie du fragment
    "void main() {\n"
    "   FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"  // Ici on définit une couleur orange (RGB = 1.0, 0.5, 0.2, 1.0 pour la coleur orange)
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
        std::cout << "Échec de création de la fenêtre GLFW" << std::endl;
        glfwTerminate();  
        return -1;
    }
    glfwMakeContextCurrent(window);  // Faire de cette fenêtre la fenêtre contextuelle active
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // Appele la fonction pour gérer les changements de taille de fenêtre

    // Initialisation de GLAD (gestion des extensions OpenGL)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {  // Charge toutes les fonctions OpenGL avec GLAD
        std::cout << "Échec d'initialisation de GLAD" << std::endl;
        return -1;
    }

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
    // free memoire -> donc optimisation

    // Boucle principale de rendu
    while (!glfwWindowShouldClose(window)) { 
        glClear(GL_COLOR_BUFFER_BIT);  // Efface l'écran avec la couleur de fond par défaut (noir)

        glUseProgram(shaderProgram);  // Utilise le programme shader pour dessiner
        glBindVertexArray(VAO);  // Lie le VAO pour qu'il soit utilisé dans le rendu
        glDrawArrays(GL_TRIANGLES, 0, 9);  // Dessine un triangle (3 sommets)
        
        glfwSwapBuffers(window);  // Échange les buffers pour afficher l'image rendue à l'écran
        glfwPollEvents();  // Vérifier les événements (ex : fermer la fenêtre, touches pressées...)
    }

    // Nettoyage des ressources après utilisation
    glDeleteVertexArrays(1, &VAO);  // Supprime le VAO
    glDeleteBuffers(1, &VBO);  // Supprime le VBO
    glDeleteProgram(shaderProgram);  // Supprime le programme shader
    glfwTerminate();  // Termine proprement GLFW

    return 0;  // Termine l'exécution du programme
}
