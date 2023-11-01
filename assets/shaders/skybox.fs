#version 330 core

uniform samplerCube skybox;

in vec3 uv;

out vec4 FragColor;

void main()
{    
    vec3 test = uv;

    FragColor = texture(skybox, uv);
    //FragColor = vec4(test, 1.0);
    //FragColor = vec4(-0.5, 0.5, -0.5, 1.0);
}