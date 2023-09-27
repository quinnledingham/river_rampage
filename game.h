#ifndef GAME_H
#define GAME_H

struct Light_Source
{
    v3 position;
    v4 color;
};

struct Camera
{
    v3 position;
    v3 target;
    v3 up;
    r32 fov;
    r32 yaw;
    r32 pitch;
};
function inline m4x4 get_view(Camera camera) { return look_at(camera.position, camera.position + camera.target, camera.up); }

struct Game_Data
{
    Light_Source light;
    Camera camera;
    Mesh water;
};

#endif //GAME_H
