#pragma once

#include "framework.h"

#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <math.h>
#include <stdlib.h>
#include <algorithm>

#include <corecrt_math_defines.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>


//struct XMFLOAT3
//{
//    float x;
//    float y;
//    float z;
//
//    XMFLOAT3() = default;
//    XMFLOAT3(float x, float y = 0, float z = 0) : x(x), y(y), z(z) {}
//
//    XMFLOAT3 operator*(float scalar) const {
//        return XMFLOAT3(x * scalar, y * scalar, z * scalar);
//    }
//};


//struct XMFLOAT4
//{
//    float x;
//    float y;
//    float z;
//    float w;
//
//    XMFLOAT4() = default;
//    XMFLOAT4(float x, float y = 0, float z = 0, float w = 0) : x(x), y(y), z(z), w(w) {}
//    
//    XMFLOAT4 operator*(float scalar) const {
//        return XMFLOAT4(x * scalar, y * scalar, z * scalar, w * scalar);
//    }
//};


template <typename T>
struct Point2
{
    T x, y;

    inline Point2<T> operator-(const Point2<T>& rhs) const
    {
        return Point2<T>{x - rhs.x, y - rhs.y};
    }
};

template <typename T>
struct Point3
{
    T x, y, z;

    Point3<T>(const T& _x = (T)0, const T& _y = (T)0, const T& _z = (T)0)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }

    inline T lengthSqr() const { return x * x + y * y + z * z; }
    inline T length() const { return (T)sqrt(lengthSqr()); }

    void normalize()
    {
        T len = length();
        x /= len;
        y /= len;
        z /= len;
    }

    inline Point3<T> operator-() const
    {
        return Point3<T>{-x, -y, -z};
    }

    inline Point3<T> operator-(const Point3<T>& rhs) const
    {
        return Point3<T>{x - rhs.x, y - rhs.y, z - rhs.z};
    }

    inline Point3<T> operator+(const Point3<T>& rhs) const
    {
        return Point3<T>{x + rhs.x, y + rhs.y, z + rhs.z};
    }

    inline Point3<T> cross(const Point3<T>& rhs) const
    {
        return Point3<T>{y* rhs.z - z * rhs.y, -x * rhs.z + z * rhs.x, x* rhs.y - y * rhs.x};
    }

    inline T dot(const Point3<T>& rhs) const
    {
        return T(x * rhs.x + y * rhs.y + z * rhs.z);
    }

    inline Point3<T> operator*(const T& a) const
    {
        return Point3<T>{x* a, y* a, z* a};
    }
};

template <typename T>
struct Point4
{
    T x, y, z, w;

    Point4<T>(const T& _x = (T)0, const T& _y = (T)0, const T& _z = (T)0, const T& _w = (T)0)
        : x(_x)
        , y(_y)
        , z(_z)
        , w(_w)
    {
    }

    Point4<T>(const Point3<T>& p, const T& _w = (T)0)
        : x(p.x)
        , y(p.y)
        , z(p.z)
        , w(_w)
    {
    }

    inline T lengthSqr() const { return x * x + y * y + z * z + w * w; }
    inline T length() const { return (T)sqrt(lengthSqr()); }

    void normalize()
    {
        T len = length();
        x /= len;
        y /= len;
        z /= len;
        w /= len;
    }

    inline Point4<T> operator*(const T& a) const
    {
        return Point4<T>{x* a, y* a, z* a, w* a};
    }

    inline Point4<T> operator+(const Point4<T>& a) const
    {
        return Point4<T>{x + a.x, y + a.y, z + a.z, w + a.w};
    }

    inline Point4<T> operator-() const
    {
        return Point4<T>{-x, -y, -z, -w};
    }

    operator Point3<T>() const
    {
        return Point3<T>{x, y, z};
    }

    T dot(const Point4<T>& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    /** Spherical interpolation of quaternions */
    inline static Point4<T> Slerp(const Point4<T>& a, const Point4<T>& b, T t)
    {
        T cosOmega = a.dot(b);
        // Work with negative b
        Point4<T> b0 = cosOmega < (T)0 ? -b : b;
        if (cosOmega < 0)
        {
            cosOmega = -cosOmega;
        }

        T scaleA, scaleB;
        // Normal case
        if (fabs((T)1 - cosOmega) > 0.0001f)
        {
            T omega = acosf(cosOmega);
            T invSin = (T)1 / sinf(omega);
            scaleA = sinf(((T)1 - t) * omega) * invSin;
            scaleB = sinf(t * omega) * invSin;
        }
        else
        {
            // Border case - linear
            scaleA = (T)1 - t;
            scaleB = t;
        }

        return a * scaleA + b0 * scaleB;
    }
};

using XMFLOAT4 = Point4<float>;
using XMFLOAT3 = Point3<float>;

struct SphereGeomBuffer
{
    DirectX::XMMATRIX m;
    XMFLOAT4 size;
};

struct Sphere
{
    Sphere() 
        : m_pSphereGeomBuffer(nullptr)
        , m_pSphereVertexBuffer(nullptr)
        , m_pSphereIndexBuffer(nullptr)
        , m_sphereIndexCount(0)
        , indexCount(0)
        , vertexCount(0)
        , SphereSteps(0)
    {}


    void GetSphereDataSize(size_t SphereSteps);
    void CreateSphere();

    HRESULT CreateVertexBuffer(ID3D11Device* m_pDevice);
    HRESULT CreateIndexBuffer(ID3D11Device* m_pDevice);
    HRESULT CreateGeometryBuffer(ID3D11Device* m_pDevice);

    void CleanupSphere();

    ID3D11Buffer*       m_pSphereGeomBuffer;
    ID3D11Buffer*       m_pSphereVertexBuffer;
    ID3D11Buffer*       m_pSphereIndexBuffer;

    UINT m_sphereIndexCount;

    size_t SphereSteps;

    std::vector<XMFLOAT3> sphereVertices{};
    std::vector<UINT16> indices{};

    size_t indexCount;
    size_t vertexCount;
};


