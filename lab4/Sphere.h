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

#include "XMFLOAT3.h"
#include "XMFLOAT4.h"


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


