#ifndef TYPES_H
#define TYPES_H

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s8 b8;
typedef s32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;
typedef r32 f32;
typedef r64 f64;

#define function        static
#define local_persist   static
#define global_variable static

#define DEG2RAD 0.0174533f
#define PI      3.14159265359f
#define EPSILON 0.00001f

#define ARRAY_COUNT(n)     (sizeof(n) / sizeof(n[0]))
#define ARRAY_MALLOC(t, n) ((t*)SDL_malloc(n * sizeof(t)))

union v2
{
    struct
    {
        r32 x, y;
    };
    struct
    {
        r32 u, v;
    };
    struct
    {
        r32 width, height;
    };
    r32 E[2];
};

v2 operator+(const v2 &l, const v2  &r) { return { l.x + r.x, l.y + r.y }; }
v2 operator+(const v2 &l, const r32 &r) { return { l.x + r, l.y + r };     }
v2 operator-(const v2 &l, const v2  &r) { return { l.x - r.x, l.y - r.y }; }
v2 operator*(const v2 &l, const v2  &r) { return { l.x * r.x, l.y * r.y }; }
v2 operator*(const v2 &l, const r32 &r) { return { l.x * r, l.y * r };     }
v2 operator/(const v2 &l, const v2  &r) { return { l.x / r.x, l.y / r.y }; }
v2 operator/(const v2 &l, const r32 &r) { return { l.x / r, l.y / r };     }
v2 operator-(const v2 &v)               { return { -v.x, -v.y }; }

void operator+=(v2 &l, const v2  &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
void operator-=(v2 &l, const v2  &r) { l.x = l.x - r.x; l.y = l.y - r.y; }
void operator-=(v2 &l, const r32 &r) { l.x = l.x - r; l.y = l.y - r;     }
void operator*=(v2 &l, const r32 &r) { l.x = l.x * r; l.y = l.y * r;     }
void operator/=(v2 &l, const v2  &r) { l.x = l.x / r.x; l.y = l.y / r.y; }
void operator/=(v2 &l, const r32 &r) { l.x = l.x / r; l.y = l.y / r;     }

r32 dot_product(const v2 &l, const v2 &r) { return (l.x * r.x) + (l.y * r.y); }
r32 length_squared(const v2 &v) { return (v.x * v.x) + (v.y * v.y); }
void log(const v2 &v) { log("v2: %f, %f", v.x, v.y); }

inline v2
normalized(const v2 &v)
{
    r32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return v;
    r32 inverse_length = 1.0f / sqrtf(len_sq);
    return { v.x * inverse_length, v.y * inverse_length };
}

inline v2
projection_onto_line(v2 v, v2 line)
{
    return line * (dot_product(v, line) / dot_product(line, line));
}

union v2s
{
    struct
    {
        s32 x, y;
    };
    struct
    {
        s32 u, v;
    };
    struct
    {
        s32 width, height;
    };
    s32 E[2];
};

v2s operator+(const v2s &l, const v2s &r) { return { l.x + r.x, l.y + r.y }; }
v2s operator+(const v2s &l, const s32 &r) { return { l.x + r, l.y + r }; }
v2s operator-(const v2s &l, const v2s &r) { return { l.x - r.x, l.y - r.y }; }
v2s operator-(const v2s &l, const int &r) { return { l.x - r, l.y - r }; }
v2s operator*(const v2s &l, const s32 &r) { return { l.x * r, l.y * r }; }

void operator+=(v2s &l, const v2s &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
void operator+=(v2s &l, const s32 &r) { l.x = l.x + r; l.y = l.y + r; }
void operator*=(v2s &l, const s32 &r) { l.x = l.x * r; l.y = l.y * r; }
bool operator==(const v2s &l, const v2s &r) { if (l.x == r.x && l.y == r.y) return true; return false; }
bool operator!=(const v2s &l, const v2s &r) { if (l.x != r.x || l.y != r.y) return true; return false; }

v2 cv2(v2s v) { return { (r32)v.x, (r32)v.y }; }
void log(const v2s &v) { log("v2s: %d, %d", v.x, v.y); }

inline v2s
normalized(const v2s &v)
{
    v2s n = {};
    
    if      (v.x > 0)  n.x = 1;
    else if (v.x == 0) n.x = 0;
    else if (v.x < 0)  n.x = -1;
    
    if      (v.y > 0)  n.y = 1;
    else if (v.y == 0) n.y = 0;
    else if (v.y < 0)  n.y = -1;
    
    return n;
}

union v3
{
    struct
    {
        r32 x, y, z;
    };
    struct
    {
        r32 r, g, b;
    };
    r32 E[3];
};

inline v3 operator+(const v3 &l, const v3  &r) { return { l.x + r.x, l.y + r.y, l.z + r.z }; }
inline v3 operator-(const v3 &l, const v3  &r) { return { l.x - r.x, l.y - r.y, l.z - r.z }; }
inline v3 operator*(const v3 &l, const v3  &r) { return { l.x * r.x, l.y * r.y, l.z * r.z }; }
inline v3 operator*(const v3 &l, float      r) { return {l.x * r, l.y * r, l.z * r}; }
inline v3 operator/(const v3 &l, const v3  &r) { return { l.x / r.x, l.y / r.y, l.z / r.z }; }
inline v3 operator/(const v3 &l, const r32 &r) { return { l.x / r, l.y / r, l.z / r }; }

inline void operator+=(v3 &l, const v3 &r) { l.x = l.x + r.x; l.y = l.y + r.y; l.z = l.z + r.z; }
inline void operator+=(v3 &l, const r32 &r) { l.x = l.x + r; l.y = l.y + r; l.z = l.z + r; }
inline void operator-=(v3 &l, const v3 &r) { l.x = l.x - r.x; l.y = l.y - r.y; l.z = l.z - r.z; }
inline void operator-=(v3 &l, const r32 &r) { l.x = l.x - r; l.y = l.y - r; l.z = l.z - r; }
inline void operator*=(v3 &l, v3 &r) { l.x *= r.x; l.y *= r.y; l.z *= r.z; }
inline bool operator==(const v3 &l, const v3 &r) { if (l.x == r.x && l.y == r.y && l.z == r.z) return true; return false; }
inline bool operator==(const v3 &v, float f) { if (v.x == f && v.y == f && v.z == f) return true; return false; }

inline r32 dot_product(const v3 &l, const v3 &r) { return (l.x * r.x) + (l.y * r.y) + (l.z * r.z); }
inline r32 length_squared(const v3 &v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z); }

inline void
normalize(v3 &v)
{
    r32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return;
    r32 inverse_length = 1.0f / sqrtf(len_sq);
    v.x *= inverse_length;
    v.y *= inverse_length;
    v.z *= inverse_length;
}

inline v3
normalized(const v3 &v)
{
    r32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return v;
    r32 inverse_length = 1.0f / sqrtf(len_sq);
    return { v.x * inverse_length, v.y * inverse_length, v.z * inverse_length };
}

inline v3
cross_product(const v3 &l, const v3 &r)
{
    return 
    {
        (l.y * r.z - l.z * r.y),
        (l.z * r.x - l.x * r.z),
        (l.x * r.y - l.y * r.x)
    };
}

union v4
{
    struct
    {
        r32 x, y, z, w;
    };
    struct
    {
        r32 r, g, b, a;
    };
    r32 E[4];
};

inline v4 operator*(const v4 &l, const v4 &r) { return { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w }; }
inline v4 operator*(const v4 &l, float     r) { return { l.x * r, l.y * r, l.z * r, l.w * r }; }
inline f32 length_squared(const v4 &v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }
inline bool operator==(const v4 &l, const v4 &r) { if (l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w) return true; return false; }

union quat
{
    struct
    {
        r32 x, y, z, w;
    };
    struct
    {
        v3 vector;
        f32 scalar;
    };
    r32 E[4];
};

inline r32 length_squared(const quat &v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }

inline quat 
operator*(const quat &l, const quat &r) 
{
    return {
        r.x * l.w + r.y * l.z - r.z * l.y + r.w * l.x,
        -r.x * l.z + r.y * l.w + r.z * l.x + r.w * l.y,
        r.x * l.y - r.y * l.x + r.z * l.w + r.w * l.z,
        -r.x * l.x - r.y * l.y - r.z * l.z + r.w * l.w
    };
}

inline v3 
operator*(const quat& q, const v3& v)
{
    return q.vector * 2.0f * dot_product(q.vector, v) + 
        v * (q.scalar * q.scalar - dot_product(q.vector, q.vector)) +
        cross_product(q.vector, v) * 2.0f * q.scalar;
}

inline quat
normalized(const quat &v)
{
    r32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return { 0, 0, 0, 1 };
    r32 inverse_length = 1.0f / sqrtf(len_sq);
    return {v.x * inverse_length, v.y * inverse_length, v.z * inverse_length, v.w * inverse_length};
}

quat get_rotation(r32 angle, const v3& axis)
{
    v3 norm = normalized(axis);
    r32 s = sinf(angle * 0.5f);
    return { norm.x * s, norm.y * s, norm.z * s, cosf(angle * 0.5f) };
}

// Returns a quat which contains the rotation between two vectors.
// The two vectors are treated like they are points in the same sphere.
function quat
from_to(const v3& from, const v3& to)
{
    v3 f = normalized(from);
    v3 t = normalized(to);
    if (f == t)
    {
        return { 0, 0, 0, 1 };
    }
    else if (f == t * -1.0f)
    {
        v3 ortho = { 1, 0, 0 };
        if (fabsf(f.y) < fabsf(f.x))
            ortho = { 0, 1, 0 };
        if (fabsf(f.z) < fabs(f.y) && fabs(f.z) < fabsf(f.x))
            ortho = { 0, 0, 1 };
        v3 axis = normalized(cross_product(f, ortho));
        return { axis.x, axis.y, axis.z, 0.0f };
    }
    v3 half = normalized(f + t);
    v3 axis = cross_product(f, half);
    return { axis.x, axis.y, axis.z, dot_product(f, half) };
}

function quat 
get_rotation_to_direction(const v3& direction, const v3& up)
{
    // Find orthonormal basis vectors
    v3 forward = normalized(direction);
    v3 norm_up = normalized(up);
    v3 right = cross_product(norm_up, forward);
    norm_up = cross_product(forward, right);
    
    quat world_to_object = from_to({ 0, 0, 1 }, forward); // From world forward to object forward
    v3 object_up = { 0, 1, 0 };
    object_up = world_to_object * object_up; // What direction is the new object up?
    quat u_to_u = from_to(object_up, norm_up); // From object up to desired up
    quat result = world_to_object * u_to_u; // Rotate to forward direction then twist to correct up
    
    return normalized(result);
}

struct m4x4
{
    r32 E[4][4];
};

function void
print_m4x4(m4x4 matrix)
{
    for (int i = 0; i < 16; i++)
    {
        s32 row = i / 4;
        s32 column = i - (row * 4);
        printf("%f ", matrix.E[row][column]);
        if ((i + 1) % 4 == 0)
            printf("\n");
    }
}

inline m4x4
get_frustum(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    if (l == r || t == b || n == f)
    {
        error("Invalid frustum");
        return {};
    }
    
    return
    {
        (2.0f * n) / (r - l), 0, 0, 0,
        0, (2.0f * n) / (t - b), 0, 0,
        (r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
        0, 0, (-2 * f * n) / (f - n), 0
    };
}

inline m4x4
perspective_projection(r32 fov, r32 aspect_ratio, r32 n, r32 f)
{
    r32 y_max = n * tanf(fov * PI / 360.0f);
    r32 x_max = y_max * aspect_ratio;
    return get_frustum(-x_max, x_max, -y_max, y_max, n, f);
}

inline m4x4
orthographic_projection(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    if (l == r || t == b || n == f)
    {
        error("orthographic_projection() Invalid arguments");
        return {};
    }
    return
    {
        2.0f / (r - l), 0, 0, 0,
        0, 2.0f / (t - b), 0, 0,
        0, 0, -2.0f / (f - n), 0,
        -((r+l)/(r-l)),-((t+b)/(t-b)),-((f+n)/(f-n)), 1
    };
}

inline m4x4
identity_m4x4()
{
    return
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

inline m4x4
look_at(const v3 &position, const v3 &target, const v3 &up)
{
    v3 f = normalized(target - position) * -1.0f;
    v3 r = cross_product(up, f);
    if (r == 0) return identity_m4x4();
    normalize(r);
    v3 u = normalized(cross_product(f, r));
    v3 t = {-dot_product(r, position), -dot_product(u, position), -dot_product(f, position)};
    
    return
    {
        r.x, u.x, f.x, 0,
        r.y, u.y, f.y, 0,
        r.z, u.z, f.z, 0,
        t.x, t.y, t.z, 1
    };
}

inline m4x4 
create_transform_m4x4(v3 position, quat rotation, v3 scale)
{
    v3 x = {1, 0, 0};
    v3 y = {0, 1, 0};
    v3 z = {0, 0, 1};
    
    x = rotation * x;
    y = rotation * y;
    z = rotation * z;
    
    x = x * scale.x;
    y = y * scale.y;
    z = z * scale.z;
    
    return
    {
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
        z.x, z.y, z.z, 0,
        position.x, position.y, position.z, 1
    };
}

//
// End of core math structs
//

//
// string
//

#include <string>

function b32
equal(const char* a, const char *b)
{
    if (a == 0 && b == 0)
        return true;
    if (a == 0 || b == 0)
        return false;
    //printf("%s == %s\n", a, b);
    int i = 0;
    do
    {
        if (a[i] != b[i])
            return false;
    } while(a[i] != 0 && b[i++] != 0);
    
    return true;
}

function u32
get_length(const char *string)
{
    if (string == 0)
        return 0;
    
    u32 length = 0;
    const char *ptr = string;
    while(*ptr != 0)
    {
        length++;
        ptr++;
    }
    return length;
}

// converts n number of chars to a string
// ex) chtos(3, a, b, c) returns "abc"
function char*
chtos(int n, ...)
{
    char* s = (char*)malloc(n + 1);
    memset(s, 0, n + 1);
    
    va_list ptr;
    va_start(ptr, n);
    for (int i = 0; i < n; i++)
    {
        s[i] = va_arg(ptr, int);
    }
    
    return s;
}

function char*
ftos(f32 f)
{
    u32 size = 64;
    char *buffer = (char*)malloc(size);
    memset(buffer, 0, size);
    u32 ret = snprintf(buffer, size, "%f", f);
    if (ret < 0)
    {
        error(0, "ftos(): failed");
        return 0;
    }
    if (ret >= size) warning(0, "ftos(): result was truncated");
    return buffer;
}


#endif //TYPES_H
