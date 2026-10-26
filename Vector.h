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

/* Vector2 */ 

// 加算
inline Vector2 operator+(const Vector2& a, const Vector2& b) {
    return { a.x + b.x, a.y + b.y };
}

// 減算
inline Vector2 operator-(const Vector2& a, const Vector2& b) {
    return { a.x - b.x, a.y - b.y };
}

// 符号反転
inline Vector2 operator-(const Vector2& v) {
    return { -v.x, -v.y };
}

// スカラー乗算
inline Vector2 operator*(const Vector2& v, float s) {
    return { v.x * s, v.y * s };
}

inline Vector2 operator*(float s, const Vector2& v) {
    return v * s;
}

// スカラー除算
inline Vector2 operator/(const Vector2& v, float s) {
    return { v.x / s, v.y / s };
}

// 複合代入
inline Vector2& operator+=(Vector2& a, const Vector2& b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Vector2& operator-=(Vector2& a, const Vector2& b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline Vector2& operator*=(Vector2& v, float s) {
    v.x *= s;
    v.y *= s;
    return v;
}

inline Vector2& operator/=(Vector2& v, float s) {
    v.x /= s;
    v.y /= s;
    return v;
}

// 比較
inline bool operator==(const Vector2& a, const Vector2& b) {
    return a.x == b.x && a.y == b.y;
}

inline bool operator!=(const Vector2& a, const Vector2& b) {
    return !(a == b);
}



/* Vector3 */

// 加算
inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

// 減算
inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

// 符号反転
inline Vector3 operator-(const Vector3& v) {
    return { -v.x, -v.y, -v.z };
}

// スカラー乗算
inline Vector3 operator*(const Vector3& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}

inline Vector3 operator*(float s, const Vector3& v) {
    return v * s;
}

// スカラー除算
inline Vector3 operator/(const Vector3& v, float s) {
    return { v.x / s, v.y / s, v.z / s };
}

// 複合代入
inline Vector3& operator+=(Vector3& a, const Vector3& b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline Vector3& operator-=(Vector3& a, const Vector3& b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

inline Vector3& operator/=(Vector3& v, float s) {
    v.x /= s;
    v.y /= s;
    v.z /= s;
    return v;
}

// 比較
inline bool operator==(const Vector3& a, const Vector3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline bool operator!=(const Vector3& a, const Vector3& b) {
    return !(a == b);
}
