#version 330 core
#extension GL_NV_uniform_buffer_std430_layout : enable

in vec3 FragPos;
in vec3 Normal; 
in vec2 uv;   

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
    sampler2D diffuse_map;
}; 

struct Light_Source {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 color;
};

struct Light {
    float f[16];
};
  
uniform vec3 viewPos;
uniform Material material;

layout (std430) uniform Lights
{
    Light lights;
};

Light_Source get_light(float a[16]) {
    Light_Source l;
    l.position = vec3(a[0],  a[1],  a[2]);
    l.ambient  = vec3(a[3],  a[4],  a[5]);
    l.diffuse  = vec3(a[6],  a[7],  a[8]);
    l.specular = vec3(a[9],  a[10], a[11]);
    l.color    = vec4(a[12], a[13], a[14], a[15]);
    return l;
}

Light_Source light = get_light(lights.f);

void main()
{
    // ambient
    vec3 ambient = light.ambient * material.ambient;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  

    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
} 