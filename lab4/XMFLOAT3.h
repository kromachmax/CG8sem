#pragma once


#include <cmath>

struct XMFLOAT3 {
    float x, y, z;

    XMFLOAT3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}

    
    XMFLOAT3 operator+(const XMFLOAT3& other) const {
        return XMFLOAT3(x + other.x, y + other.y, z + other.z);
    }

    XMFLOAT3 operator-(const XMFLOAT3& other) const {
        return XMFLOAT3(x - other.x, y - other.y, z - other.z);
    }

    XMFLOAT3 operator*(const XMFLOAT3& other) const {
        return XMFLOAT3(x * other.x, y * other.y, z * other.z);
    }

    XMFLOAT3 operator/(const XMFLOAT3& other) const {
        return XMFLOAT3(x / other.x, y / other.y, z / other.z);
    }

    XMFLOAT3 operator*(float scalar) const {
        return XMFLOAT3(x * scalar, y * scalar, z * scalar);
    }

    XMFLOAT3 operator/(float scalar) const {
        return XMFLOAT3(x / scalar, y / scalar, z / scalar);
    }

    friend XMFLOAT3 operator*(float scalar, const XMFLOAT3& vec) {
        return vec * scalar;
    }


    XMFLOAT3& operator+=(const XMFLOAT3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    XMFLOAT3& operator-=(const XMFLOAT3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    XMFLOAT3& operator*=(const XMFLOAT3& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    XMFLOAT3& operator/=(const XMFLOAT3& other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    XMFLOAT3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    XMFLOAT3& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    bool operator==(const XMFLOAT3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const XMFLOAT3& other) const {
        return !(*this == other);
    }

    float LengthSquared() const {
        return x * x + y * y + z * z;
    }

    float Length() const {
        return std::sqrt(LengthSquared());
    }

    XMFLOAT3 Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            return *this / len;
        }
        return *this;
    }

    void Normalize() {
        *this = Normalized();
    }

    float Dot(const XMFLOAT3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    XMFLOAT3 Cross(const XMFLOAT3& other) const {
        return XMFLOAT3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
};