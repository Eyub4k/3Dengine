// Eyub Celebioglu
/*
#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    vec4 textureColor = texture(ourTexture, TexCoord);
    
    // Lumière ambiante
    vec3 ambient = 0.3 * lightColor;
    
    // Lumière diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Lumière finale avec texture
    vec3 result = (ambient + diffuse) * vec3(textureColor);
    
    FragColor = vec4(result, textureColor.a);
}
*/

#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}