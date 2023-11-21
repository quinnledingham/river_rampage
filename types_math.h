#ifndef TYPES_MATH_H
#define TYPES_MATH_H

//
// v2
//

inline v2 operator+(const v2 &l, const v2  &r) { return { l.x + r.x, l.y + r.y }; }
inline v2 operator+(const v2 &l, const r32 &r) { return { l.x + r  , l.y + r   }; }
inline v2 operator-(const v2 &l, const v2  &r) { return { l.x - r.x, l.y - r.y }; }
inline v2 operator*(const v2 &l, const v2  &r) { return { l.x * r.x, l.y * r.y }; }
inline v2 operator*(const v2 &l, const r32 &r) { return { l.x * r  , l.y * r   }; }
inline v2 operator/(const v2 &l, const v2  &r) { return { l.x / r.x, l.y / r.y }; }
inline v2 operator/(const v2 &l, const r32 &r) { return { l.x / r  , l.y / r   }; }
inline v2 operator-(const v2 &v)               { return { -v.x     , -v.y      }; }

inline void operator+=(v2 &l, const v2  &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
inline void operator-=(v2 &l, const v2  &r) { l.x = l.x - r.x; l.y = l.y - r.y; }
inline void operator-=(v2 &l, const r32 &r) { l.x = l.x - r;   l.y = l.y - r;   }
inline void operator*=(v2 &l, const r32 &r) { l.x = l.x * r;   l.y = l.y * r;   }
inline void operator/=(v2 &l, const v2  &r) { l.x = l.x / r.x; l.y = l.y / r.y; }
inline void operator/=(v2 &l, const r32 &r) { l.x = l.x / r;   l.y = l.y / r;   }

inline r32 dot_product(const v2 &l, const v2 &r) { return (l.x * r.x) + (l.y * r.y); }
inline r32 length_squared(const v2 &v) { return (v.x * v.x) + (v.y * v.y); }
inline void log(const v2 &v) { log("v2: %f, %f", v.x, v.y); }
inline v2s cv2(v2 v) { return { (s32)v.x, (s32)v.y }; }

inline 
v2 pow(const v2 &v, u32 exponent)
{
    v2 result = v;
    for (u32 i = 1; i < exponent; i++) { result.x *= v.x; result.y *= v.y; }
    return result;
}

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

inline f32
magnitude(const v2 &v)
{
    f32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return 1;
    return (f32)sqrt(len_sq);
}

inline f32
angle_between(const v2 &a, const v2 &b)
{
    f32 dot = (dot_product(a, b));
    f32 cos_theta = dot / (magnitude(a) * magnitude(b));
    
    if      (cos_theta >  1.0) cos_theta =  1.0;
    else if (cos_theta < -1.0) cos_theta = -1.0;
    
    f32 result = acosf(cos_theta);
    return result;
}

//
// v2s 
//

inline v2s operator+(const v2s &l, const v2s &r) { return { l.x + r.x, l.y + r.y }; }
inline v2s operator+(const v2s &l, const s32 &r) { return { l.x + r,   l.y + r   }; }
inline v2s operator-(const v2s &l, const v2s &r) { return { l.x - r.x, l.y - r.y }; }
inline v2s operator-(const v2s &l, const int &r) { return { l.x - r,   l.y - r   }; }
inline v2s operator*(const v2s &l, const s32 &r) { return { l.x * r,   l.y * r   }; }

inline void operator+=(v2s &l, const v2s &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
inline void operator+=(v2s &l, const s32 &r) { l.x = l.x + r;   l.y = l.y + r;   }
inline void operator*=(v2s &l, const s32 &r) { l.x = l.x * r;   l.y = l.y * r;   }
inline bool operator==(const v2s &l, const v2s &r) { if (l.x == r.x && l.y == r.y) return true; return false; }
inline bool operator!=(const v2s &l, const v2s &r) { if (l.x != r.x || l.y != r.y) return true; return false; }

inline v2 cv2(v2s v) { return { (r32)v.x, (r32)v.y }; }
inline void log(const v2s &v) { log("v2s: %d, %d", v.x, v.y); }

inline v2s
normalized(const v2s &v)
{
    v2s n = {};
    
    if      (v.x >  0) n.x =  1;
    else if (v.x == 0) n.x =  0;
    else if (v.x <  0) n.x = -1;
    
    if      (v.y >  0) n.y =  1;
    else if (v.y == 0) n.y =  0;
    else if (v.y <  0) n.y = -1;
    
    return n;
}

//
// v3
//

inline v3 operator+(const v3 &l, const v3  &r) { return { l.x + r.x, l.y + r.y, l.z + r.z }; }
inline v3 operator-(const v3 &l, const v3  &r) { return { l.x - r.x, l.y - r.y, l.z - r.z }; }
inline v3 operator*(const v3 &l, const v3  &r) { return { l.x * r.x, l.y * r.y, l.z * r.z }; }
inline v3 operator*(const v3 &l, float      r) { return {l.x * r,    l.y * r,   l.z * r   }; }
inline v3 operator/(const v3 &l, const v3  &r) { return { l.x / r.x, l.y / r.y, l.z / r.z }; }
inline v3 operator/(const v3 &l, const r32 &r) { return { l.x / r,   l.y / r,   l.z / r   }; }
inline v3 operator-(const v3 &v)               { return {-v.x    ,  -v.y    ,  -v.z       }; }

inline void operator+=(v3 &l, const v3 &r)  { l.x = l.x + r.x; l.y = l.y + r.y; l.z = l.z + r.z; }
inline void operator+=(v3 &l, const r32 &r) { l.x = l.x + r;   l.y = l.y + r;   l.z = l.z + r;   }
inline void operator-=(v3 &l, const v3 &r)  { l.x = l.x - r.x; l.y = l.y - r.y; l.z = l.z - r.z; }
inline void operator-=(v3 &l, const r32 &r) { l.x = l.x - r;   l.y = l.y - r;   l.z = l.z - r;   }
inline void operator*=(v3 &l, v3 &r)        { l.x *= r.x;      l.y *= r.y;      l.z *= r.z;      }
inline bool operator==(const v3 &l, const v3 &r) { if (l.x == r.x && l.y == r.y && l.z == r.z) return true; return false; }
inline bool operator==(const v3 &v, float f)     { if (v.x == f   && v.y == f   && v.z == f)   return true; return false; }

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

inline f32
magnitude(const v3 &v)
{
    f32 len_sq = length_squared(v);
    return (f32)sqrt(len_sq);
}

inline f32
angle_between(const v3 &a, const v3 &b)
{
    return acosf((dot_product(a, b)) / (magnitude(a) * magnitude(b)));
}

inline v3
projection_onto_line(v3 v, v3 line)
{
    return line * (dot_product(v, line) / dot_product(line, line));
}

inline v3
average(const v3 &l, const v3 &r) {
    v3 result = {};
    result.x = (l.x + r.x) / 2.0f;
    result.y = (l.y + r.y) / 2.0f;
    result.z = (l.x + r.z) / 2.0f;
    return result;
}


//
// v4
//

inline v4 operator*(const v4 &l, const v4 &r) { return { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w }; }
inline v4 operator*(const v4 &l, float     r) { return { l.x * r, l.y * r, l.z * r, l.w * r }; }
inline f32 length_squared(const v4 &v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }
inline bool operator==(const v4 &l, const v4 &r) { if (l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w) return true; return false; }

//
// quat
//

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

inline quat 
get_rotation(r32 angle, const v3& axis)
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

inline quat 
get_rotation_to_direction(const v3& direction, const v3& up)
{
    // Find orthonormal basis vectors
    v3 forward = normalized(direction);
    v3 norm_up = normalized(up);
    v3 right = cross_product(norm_up, forward);
    norm_up = cross_product(forward, right);
    
    quat world_to_object = from_to({ 0, 0, 1 }, forward); // From world forward to object forward
    v3 object_up = { 0, 1, 0 };
    object_up = world_to_object * object_up;   // What direction is the new object up?
    quat u_to_u = from_to(object_up, norm_up); // From object up to desired up
    quat result = world_to_object * u_to_u;    // Rotate to forward direction then twist to correct up
    
    return normalized(result);
}

//
// m4x4
//

inline void
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

#endif // TYPES_MATH_H