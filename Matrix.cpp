
#include "Matrix.h"

#define NOMINMAX
#include <algorithm>

#include <cmath>

#include <cassert>


Vector3 Lerp(const Vector3& a, const Vector3& b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }
Vector4 Lerp(const Vector4& a, const Vector4& b, float t)
{
	return {
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t,
		a.w + (b.w - a.w) * t
	};
};
// スムーズ
float LerpAngle(float a, float b, float t) {
	float diff = b - a;

	// -π ～ +π に正規化（最短回転）
	while (diff > 3.141592f)
		diff -= 6.283185f;
	while (diff < -3.141592f)
		diff += 6.283185f;

	return a + diff * t;
}

float Smoothstep(float a, float b, float t)
{
	// 3t^2 - 2t^3 または t^2( 3 - 2t )
	return t * t * (3 - 2 * t);
}

float Follow(float value, float target, float speed, float deltaTime)
{
	float t = std::clamp(speed * deltaTime, 0.0f, 1.0f);

	return value + (target- value) * t;
}

Vector2 Follow(Vector2 value, Vector2 target, float speed, float deltaTime) {
	float t = std::clamp(speed * deltaTime, 0.0f, 1.0f);

	Vector2 diff = Subtract(target, value);

	return {
		value.x + diff.x * t,
		value.y + diff.y * t
	};
}

Vector3 Follow(Vector3 value, Vector3 target, float speed, float deltaTime) {

	float t = std::clamp(speed * deltaTime, 0.0f, 1.0f);

	return value + (target - value) * t;
};


// 行列の計算とか

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 result{};

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
		}
	}

	return result;
}

Vector3 Multiply(const Vector3& v, const Matrix4x4& m) {
	Vector3 result;

	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + 1.0f * m.m[3][0];

	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + 1.0f * m.m[3][1];

	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + 1.0f * m.m[3][2];

	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + 1.0f * m.m[3][3];

	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}

	return result;
}

Vector3 Multiply(float scalar, const Vector3& v) { return {v.x * scalar, v.y * scalar, v.z * scalar}; }

// 単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1.0f;
	}
	return result;
};

// 1. 平行移動行列
Matrix4x4 makeTranslationMatrix(const Vector3& translation) {
	Matrix4x4 result = MakeIdentity4x4();

	result.m[3][0] = translation.x;
	result.m[3][1] = translation.y;
	result.m[3][2] = translation.z;

	return result;
}

// 2. 拡大縮小行列
Matrix4x4 makeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	return result;
}

Matrix4x4 MakeRotationMatrix(const Vector3& rotation)
{
	// 回転X
	float cx = cosf(rotation.x);
	float sx = sinf(rotation.x);
	Matrix4x4 Rx{ 1, 0, 0, 0, 0, cx, sx, 0, 0, -sx, cx, 0, 0, 0, 0, 1 };

	// 回転Y
	float cy = cosf(rotation.y);
	float sy = sinf(rotation.y);
	Matrix4x4 Ry{ cy, 0, -sy, 0, 0, 1, 0, 0, sy, 0, cy, 0, 0, 0, 0, 1 };

	// 回転Z
	float cz = cosf(rotation.z);
	float sz = sinf(rotation.z);
	Matrix4x4 Rz{ cz, sz, 0, 0, -sz, cz, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	// 合成
	return Multiply(Rx, Multiply(Ry,Rz));

}

// 3. ベクトルの変換
Vector3 Transform(const Vector3& point, const Matrix4x4& matrix) {

	Vector3 result;

	result.x = point.x * matrix.m[0][0] + point.y * matrix.m[1][0] + point.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = point.x * matrix.m[0][1] + point.y * matrix.m[1][1] + point.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = point.x * matrix.m[0][2] + point.y * matrix.m[1][2] + point.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];

	float w = point.x * matrix.m[0][3] + point.y * matrix.m[1][3] + point.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	
	 // 0対策
	if (fabsf(w) < 0.00001f) {
		return {0.0f, 0.0f, 0.0f};
	}

	assert(w != 0.0f);

	result.x /= w;
	result.y /= w;
	result.z /= w;

	return result;
}

// ３次元アフィン変換
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation) {

	// スケール
	Matrix4x4 S{scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0, 0, 0, 1};

	// 回転X
	float cx = cosf(rotation.x);
	float sx = sinf(rotation.x);
	Matrix4x4 Rx{1, 0, 0, 0, 0, cx, sx, 0, 0, -sx, cx, 0, 0, 0, 0, 1};

	// 回転Y
	float cy = cosf(rotation.y);
	float sy = sinf(rotation.y);
	Matrix4x4 Ry{cy, 0, -sy, 0, 0, 1, 0, 0, sy, 0, cy, 0, 0, 0, 0, 1};

	// 回転Z
	float cz = cosf(rotation.z);
	float sz = sinf(rotation.z);
	Matrix4x4 Rz{cz, sz, 0, 0, -sz, cz, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	// 平行移動
	Matrix4x4 T{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translation.x, translation.y, translation.z, 1};

	// 合成
	Matrix4x4 world = Multiply(S, Multiply(Rx, Multiply(Ry, Multiply(Rz, T))));

	return world;
}
Matrix4x4 MakeRotateZMatrix(float radian) {

	Matrix4x4 result{};

	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}
// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}
Vector2 Add(const Vector2& v1, const Vector2& v2) {
	Vector2 result{};
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	return result;
}

// 引き算
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

Vector2 Subtract(const Vector2& v1, const Vector2& v2)
{
	Vector2 result{};
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	return result;
}


// 　スカラー倍
Vector3 Multiply(const Vector3& v, float scalar) {
	Vector3 result{};
	result.x = v.x * scalar;
	result.y = v.y * scalar;
	result.z = v.z * scalar;
	return result;
}

// 　内積
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

// 長さ
float Length(const Vector3& v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }


float Normalize(float value, float min, float max) {
	return (value - min) / (max - min);
}
// 正規化
Vector3 Normalize(const Vector3& v) {

	Vector3 result{};
	float length = Length(v);
	if (length != 0.0f) {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	}
	return result;
}

Vector4 Normalize(const Vector4& v)
{
	float length =
		std::sqrt(
			v.x * v.x +
			v.y * v.y +
			v.z * v.z +
			v.w * v.w
		);

	if (length == 0.0f)
	{
		return { 0,0,0,1 };
	}

	return {
		v.x / length,
		v.y / length,
		v.z / length,
		v.w / length
	};
}

Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;

	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {
	Vector3 result{
	    v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
	    v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
	    v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2],
	};
	return result;
}

Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 a = m;
	Matrix4x4 inv{};

	// 単位行列
	for (int i = 0; i < 4; i++) {
		inv.m[i][i] = 1.0f;
	}

	for (int i = 0; i < 4; i++) {
		float pivot = a.m[i][i];

		// 0対策（超簡易）
		if (pivot == 0.0f)
			return Matrix4x4{};

		float invPivot = 1.0f / pivot;

		// 行を正規化
		for (int j = 0; j < 4; j++) {
			a.m[i][j] *= invPivot;
			inv.m[i][j] *= invPivot;
		}

		// 他の行を消す
		for (int k = 0; k < 4; k++) {
			if (k == i)
				continue;

			float factor = a.m[k][i];

			for (int j = 0; j < 4; j++) {
				a.m[k][j] -= factor * a.m[i][j];
				inv.m[k][j] -= factor * inv.m[i][j];
			}
		}
	}

	return inv;
};
float Distance(const Vector2& a, const Vector2& b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;

	return std::sqrt(dx * dx + dy * dy);
}

float Distance(const Vector3& a, const Vector3& b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float dz = b.z - a.z;

	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Vector3 Project(const Vector3 worldPosition, float viewport, float viewportY, float viewportWidth, float viewportHeight, const Matrix4x4& viewProjection) {

	// ViewProjection変換
	Vector3 ndc = Transform(worldPosition, viewProjection);

	Vector3 result;

	// NDC(-1～1) → Screen
	result.x = (ndc.x + 1.0f) * 0.5f * viewportWidth + viewport;
	result.y = (1.0f - ndc.y) * 0.5f * viewportHeight + viewportY;
	result.z = ndc.z;

	return result;
}

//1. 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ)
{
	Matrix4x4 m{};

	float f = 1.0f / tanf(fovY * 0.5f);

	m.m[0][0] = f / aspect;
	m.m[1][1] = f;
	m.m[2][2] = farZ / (farZ - nearZ);
	m.m[2][3] = 1.0f;
	m.m[3][2] = (-nearZ * farZ) / (farZ - nearZ);

	return m;
};


Matrix4x4 MakeOrthographicMatrix(
	float left,
	float top,
	float right,
	float bottom,
	float nearClip,
	float farClip) {

	Matrix4x4 result{};

	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom); // ※ 引数順（left, top, right, bottom）に合わせる
	result.m[2][2] = 1.0f / (farClip - nearClip);

	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;


	/*result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;*/

	return result;
}

// 3.ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 m{};

	m.m[0][0] = width * 0.5f;
	m.m[1][1] = -height * 0.5f; // Y反転（画面座標）

	m.m[2][2] = maxDepth - minDepth;

	m.m[3][0] = left + width * 0.5f;
	m.m[3][1] = top + height * 0.5f;
	m.m[3][2] = minDepth;

	m.m[3][3] = 1.0f;

	return m;
};


Vector3 CatmullRomInterpolation(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t) {

	const float s = 0.5f;

	float t2 = t * t;
	float t3 = t2 * t;

	Vector3 e0 = {2.0f * p1.x, 2.0f * p1.y, 2.0f * p1.z};

	Vector3 e1 = {p2.x - p0.x, p2.y - p0.y, p2.z - p0.z};

	Vector3 e2 = {2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x, 2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y, 2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z};

	Vector3 e3 = {-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x, -p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y, -p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z};

	Vector3 result;

	result.x = s * (e3.x * t3 + e2.x * t2 + e1.x * t + e0.x);
	result.y = s * (e3.y * t3 + e2.y * t2 + e1.y * t + e0.y);
	result.z = s * (e3.z * t3 + e2.z * t2 + e1.z * t + e0.z);

	return result;
};
Vector3 CatmullRomPosition(const std::vector<Vector3>& points, float t) {

	assert(points.size() >= 4 && "制御点は4点以上必要です");

	// 区間数は制御点の数-1
	size_t division = points.size() - 1;
	// 1区間の長さ(全体を1.0とした割合)
	float areaWidth = 1.0f / division;

	// 区間内の始点を0.0f、終点を1.0fとしたときの現在位置
	// float t_2 = std::fmod(t, areaWidth) * division;
	//  下限(0.0f)と上限(1.0f)の範囲に収める
	// t_2 = std::clamp(t_2,0.0f,1.0f);

	// 区間番号
	// size_t index = static_cast<size_t>(t / areaWidth);

	// 区間番号
	float fIndex = t / areaWidth;
	size_t index = static_cast<size_t>(fIndex);

	// 末端クランプ
	if (index >= division) {
		index = division - 1;
	}

	// 区間内t（0〜1）
	float t_2 = fIndex - index;

	// 範囲制限
	if (index >= division) {
		index = division - 1;
	}

	size_t index0 = (index == 0) ? 0 : index - 1;
	size_t index1 = index;
	size_t index2 = index + 1;
	size_t index3 = index + 2;

	//// 最初の区間のｐ0はp1を重複使用する
	// if (index == 0) {
	//	index0 = index1;
	// };
	// if (index3 >= points.size()) {
	//	index3 = index2;
	// }
	if (index3 >= points.size()) {
		index3 = points.size() - 1;
	}
	// 4点の座標
	const Vector3& p0 = points[index0];
	const Vector3& p1 = points[index1];
	const Vector3& p2 = points[index2];
	const Vector3& p3 = points[index3];

	return CatmullRomInterpolation(p0, p1, p2, p3, t_2);
	// assert(points.size() >= 4 && "制御点は4点以上必要です");

	// size_t n = points.size();
	// t = std::clamp(t, 0.0f, 1.0f);

	//// 閉曲線用：制御点の数で区間を区切る（これが大事）
	//// 小さい輪っか対策：少しゆったりめにスケーリング
	// float t_scaled = t * (n - 0.5f); // 少し調整
	// size_t index = static_cast<size_t>(t_scaled);
	// float t_2 = t_scaled - index; // 区間内 t [0,1)

	//// ループ対応（モジュロ）
	// auto modIndex = [&](int i) -> size_t { return (i % n + n) % n; };

	// size_t i0 = modIndex(static_cast<int>(index) - 1);
	// size_t i1 = modIndex(static_cast<int>(index));
	// size_t i2 = modIndex(static_cast<int>(index) + 1);
	// size_t i3 = modIndex(static_cast<int>(index) + 2);

	// const Vector3& p0 = points[i0];
	// const Vector3& p1 = points[i1];
	// const Vector3& p2 = points[i2];
	// const Vector3& p3 = points[i3];

	// return CatmullRomInterpolation(p0, p1, p2, p3, t_2);
}

// 一区間内
Vector3 CatmullSegment(const std::vector<Vector3>& points, int index, float t) {
	assert(index >= 0);
	assert(index < points.size() - 1);

	int i0 = (index == 0) ? 0 : index - 1;
	int i1 = index;
	int i2 = index + 1;
	int i3 = (std::min)(index + 2, (int)points.size() - 1);

	return CatmullRomInterpolation(points[i0], points[i1], points[i2], points[i3], t);
}

Matrix4x4 MakeRotateMatrix(const Vector4& q)
{
	Matrix4x4 m{};

	float x = q.x;
	float y = q.y;
	float z = q.z;
	float w = q.w;

	// 行ベクトル用に転置した形
	m.m[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
	m.m[0][1] = 2.0f * x * y - 2.0f * z * w;   // 符号反転
	m.m[0][2] = 2.0f * x * z + 2.0f * y * w;   // 符号反転
	m.m[0][3] = 0.0f;

	m.m[1][0] = 2.0f * x * y + 2.0f * z * w;   // 符号反転
	m.m[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z;
	m.m[1][2] = 2.0f * y * z - 2.0f * x * w;   // 符号反転
	m.m[1][3] = 0.0f;

	m.m[2][0] = 2.0f * x * z - 2.0f * y * w;   // 符号反転
	m.m[2][1] = 2.0f * y * z + 2.0f * x * w;   // 符号反転
	m.m[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;

	return m;
	
}

Vector4 Transform(const Vector4& vector, const Matrix4x4& matrix) {

	Vector4 result{};

	result.x =
		vector.x * matrix.m[0][0] +
		vector.y * matrix.m[1][0] +
		vector.z * matrix.m[2][0] +
		vector.w * matrix.m[3][0];

	result.y =
		vector.x * matrix.m[0][1] +
		vector.y * matrix.m[1][1] +
		vector.z * matrix.m[2][1] +
		vector.w * matrix.m[3][1];

	result.z =
		vector.x * matrix.m[0][2] +
		vector.y * matrix.m[1][2] +
		vector.z * matrix.m[2][2] +
		vector.w * matrix.m[3][2];

	result.w =
		vector.x * matrix.m[0][3] +
		vector.y * matrix.m[1][3] +
		vector.z * matrix.m[2][3] +
		vector.w * matrix.m[3][3];

	return result;
}


// 転置
Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result{};

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m.m[j][i];
		}
	}

	return result;
}