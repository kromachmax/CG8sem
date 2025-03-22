#pragma once


#include <cmath>

#include "XMFLOAT3.h"

struct XMFLOAT4
{
    float x, y, z, w;

    XMFLOAT4(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f, float w_ = 0.0f) : x(x_), y(y_), z(z_), w(w_) {}
    XMFLOAT4(const XMFLOAT3& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}

    XMFLOAT4 operator+(const XMFLOAT4& other) const {
        return XMFLOAT4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    XMFLOAT4 operator-(const XMFLOAT4& other) const {
        return XMFLOAT4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    XMFLOAT4 operator*(const XMFLOAT4& other) const {
        return XMFLOAT4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    XMFLOAT4 operator/(const XMFLOAT4& other) const {
        return XMFLOAT4(x / other.x, y / other.y, z / other.z, w / other.w); 
    }

    XMFLOAT4 operator*(float scalar) const {
        return XMFLOAT4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    XMFLOAT4 operator/(float scalar) const {
        return XMFLOAT4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    friend XMFLOAT4 operator*(float scalar, const XMFLOAT4& vec) {
        return vec * scalar;
    }
    
    XMFLOAT4& operator+=(const XMFLOAT4& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    XMFLOAT4& operator-=(const XMFLOAT4& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    XMFLOAT4& operator*=(const XMFLOAT4& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    XMFLOAT4& operator/=(const XMFLOAT4& other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    XMFLOAT4& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    XMFLOAT4& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    bool operator==(const XMFLOAT4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    bool operator!=(const XMFLOAT4& other) const {
        return !(*this == other);
    }

    float LengthSquared() const {
        return x * x + y * y + z * z + w * w;
    }

    float Length() const {
        return std::sqrt(LengthSquared());
    }

    XMFLOAT4 Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            return *this / len;
        }
        return *this;
    }

    void Normalize() {
        *this = Normalized();
    }

    float Dot(const XMFLOAT4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    XMFLOAT3 ToFloat3() const {
        return XMFLOAT3(x, y, z);
    }
};