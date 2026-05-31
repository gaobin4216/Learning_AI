#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform float time;

out vec4 FragColor;

void main() {
    // Ambient
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Blinn-Phong)
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    // Rim light (edge glow)
    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = pow(rim, 3.0);
    vec3 rimColor = rim * objectColor * 0.4;

    // Combine
    vec3 result = (ambient + diffuse + specular) * objectColor + rimColor;

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
