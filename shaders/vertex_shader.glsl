// Eyub Celebioglu
#version 330 core

layout(location = 0) in vec3 aPos;       // position du sommet
layout(location = 1) in vec3 aNormal;    // pormal du sommet
layout(location = 2) in vec2 aTexCoord;  // coord de texture

out vec3 FragPos;        // position fragment dans l'espace monde
out vec3 Normal;         // normal fragment
out vec2 TexCoord;       // coord de texture fragment

uniform mat4 model;      // matrice modele
uniform mat4 view;       // matrice vue
uniform mat4 projection; // matrice projection

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // calcule de la position dans l'espace monde
    Normal = mat3(transpose(inverse(model))) * aNormal; // calcule\ de la normale dans l'espace monde
    TexCoord = aTexCoord; // on passe les coord de texture
    gl_Position = projection * view * vec4(FragPos, 1.0); // transformation vers l'espace de projection
}
