#pragma once

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Vector4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};


// Vector3
inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline Vector3 operator*(const Vector3& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}

inline Vector3 operator*(float s, const Vector3& v) {
    return v * s;
}