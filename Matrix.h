#pragma once
#include "Vector.h"
#include <vector>

// 行列
struct Matrix4x4
{
    float m[4][4];
};

// 線形補完
Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
Vector4 Lerp(const Vector4& a, const Vector4& b, float t);

float LerpAngle(float a, float b, float t);

// なめらかな補間
float Smoothstep(float a, float b, float t);


// targetがvalueにゆっくり追従 (線形補完
float  Follow(float value, float target, float speed, float deltaTime);

// targetがvalueにゆっくり追従 (線形補完
Vector2 Follow(Vector2 value, Vector2 target, float speed, float deltaTime);

// targetがvalueにゆっくり追従(線形補完
Vector3 Follow(Vector3 value, Vector3 target, float speed, float deltaTime);


    // 行列の計算とか
Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b);

Vector3 Multiply(const Vector3& v, const Matrix4x4& m);

// スカラー倍
Vector3 Multiply(float scalar, const Vector3& v);

    // 単位行列の作成
Matrix4x4 MakeIdentity4x4();

// 1. 平行移動行列
Matrix4x4 makeTranslationMatrix(const Vector3& translation);
// 2. 拡大縮小行列
Matrix4x4 makeScaleMatrix(const Vector3& scale);
// 3. 回転行列
Matrix4x4 MakeRotationMatrix(const Vector3& rotation);

// 3. ベクトルの変換
Vector3 Transform(const Vector3& point, const Matrix4x4& matrix);

// ３次元アフィン変換
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation);

// z軸回転行列生成
Matrix4x4 MakeRotateZMatrix(float radian);

// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2);
Vector2 Add(const Vector2& v1, const Vector2& v2);

// 引き算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);
Vector2 Subtract(const Vector2& v1, const Vector2& v2);

// 　スカラー倍
Vector3 Multiply(const Vector3& v, float scalar);

// 　内積
float Dot(const Vector3& v1, const Vector3& v2);
// 長さ
float Length(const Vector3& v);


// 正規化
float Normalize(float value, float min, float max);

// 正規化
Vector3 Normalize(const Vector3& v);

// 正規化
Vector4 Normalize(const Vector4& v);


Vector3 Cross(const Vector3& v1, const Vector3& v2);

    // ベクトル変換
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

/// 逆行列
Matrix4x4 Inverse(const Matrix4x4& m);

// 転置
Matrix4x4 Transpose(const Matrix4x4& m);

// vector2同士の距離
float Distance(const Vector2& a, const Vector2& b);

float Distance(const Vector3& a, const Vector3& b);

//1. 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ);


Matrix4x4 MakeOrthographicMatrix(
	float left,
	float top,
	float right,
	float bottom,
	float nearClip,
	float farClip);

// 3.ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

    // ワールドスクリーン変換
Vector3 Project(const Vector3 worldPosition, float viewport, float viewportY, float viewportWidth, float viewportHeight, const Matrix4x4& viewProjection);

Vector3 CatmullRomInterpolation(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t);
Vector3 CatmullRomPosition(const std::vector<Vector3>& points, float t);
Vector3 CatmullSegment(const std::vector<Vector3>& points, int index, float t);

Matrix4x4 MakeRotateMatrix(const Vector4& quaternion);


Vector4 Transform(const Vector4& vector, const Matrix4x4& matrix);