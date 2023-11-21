function void
update_particles_model_buffer(Particles *particles)
{
    m4x4 *models = ARRAY_MALLOC(m4x4, particles->count);
    r32 *lifes = ARRAY_MALLOC(r32, particles->count);

    //m4x4 models[particles->count];
    //r32 lifes[particles->count];

    u32 models_index = 0;
    for (u32 i = 0; i < particles->max; i++)
    {
        if (particles->data[i].life <= 0.0f) 
            continue;
        v3 dim = { 
            particles->data[i].weight, 
            particles->data[i].weight, 
            particles->data[i].weight 
        };
        m4x4 model = create_transform_m4x4(particles->data[i].position, get_rotation(0.0f, { 0, 1, 0 }), dim);
        if (particles->data[i].life < 0)
            log("particle life below 0 (%d)", i);
        lifes[models_index] = particles->data[i].life;
        models[models_index++] = model;
    }

    if (models_index != particles->count)
        log("models != count (%d != %d)", models_index, particles->count);

    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles->max * sizeof(m4x4), NULL);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles->count * sizeof(m4x4), &models[0]);

    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_life_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles->max * sizeof(r32), NULL);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles->count * sizeof(r32), &lifes[0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    platform_free(models);
    platform_free(lifes);
}

function void
init_particles(Particles *particles, u32 particles_max)
{
    //init_circle_mesh(&particles->mesh);
    particles->mesh = get_sphere_mesh(1.0f, 40, 40);
    particles->max = particles_max;
    particles->data = ARRAY_MALLOC(Particle, particles_max);
    SDL_memset(particles->data, 0, sizeof(Particle) * particles_max);

    //glGenVertexArrays(1, &particles->mesh.vao); generated in init_mesh()
    glBindVertexArray(particles->mesh.vao);
    glGenBuffers(1, &particles->opengl_buffer);
    glGenBuffers(1, &particles->opengl_life_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_buffer);
    glBufferData(GL_ARRAY_BUFFER, particles->max * sizeof(m4x4), NULL, GL_DYNAMIC_DRAW);

    // create a 4x4 matrix in the buffer using 4 v4s
    glEnableVertexAttribArray(3); 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)0);
    glEnableVertexAttribArray(4); 
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)(1 * sizeof(v4)));
    glEnableVertexAttribArray(5); 
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)(2 * sizeof(v4)));
    glEnableVertexAttribArray(6); 
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)(3 * sizeof(v4)));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindBuffer(GL_ARRAY_BUFFER, particles->opengl_life_buffer);
    glBufferData(GL_ARRAY_BUFFER, particles->max * sizeof(r32), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(7); 
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(r32), (void*)0);
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);
}

function void
update_particles(Particles *particles, r32 frame_time_s)
{
    r32 delta_life = frame_time_s;

    for (u32 i = 0; i < particles->max; i++) {
        Particle *part = &particles->data[i];

        if (part->life > 0.0f)  {
            part->life -= delta_life;
            if (part->life <= 0.0f) 
                particles->count--;

            //part->position += normalized(part->direction) * 0.01f;
            
            if (part->weight > 0) {
                part->weight -= 0.001f;
                if (part->weight < 0)
                    part->weight = 0;
            }
        }
    }

    update_particles_model_buffer(particles);
}

Particles global_particles = {};

void update_particles(r32 frame_time_s) {
    update_particles(&global_particles, frame_time_s);
}

void 
add_particle(v3 position, v3 direction, r32 weight)
{
    Particles *particles = &global_particles;

    Particle *part = 0;
    for (u32 i = 0; i < particles->max; i++) {
        if (particles->data[i].life <= 0.0f) {
            part = &particles->data[i];
            break;
        }
    }

    if (part == 0) 
        return; // reached max particles

    part->life = 5.0f;
    part->position = position;
    part->direction = direction;
    part->weight = weight;
    particles->count++;
}

void
draw_particles(Assets *assets, r32 seconds)
{
    Particles *particles = &global_particles;

    local_persist Shader *shader = {};
    if (shader == 0) 
        shader = find_shader(assets, "PARTICLE");
    u32 handle = use_shader(shader);
    
    v4 color = { 255, 255, 255, 1 };
    glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&color);
    platform_uniform_f32(handle, "time", seconds);
    glBindVertexArray(particles->mesh.vao); 
    glDrawElementsInstanced(GL_TRIANGLES, particles->mesh.indices_count, GL_UNSIGNED_INT, 0, particles->count);
    glBindVertexArray(0);

    //log("%d", particles->count);
}

