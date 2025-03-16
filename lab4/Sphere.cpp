#include "Sphere.h"

void Sphere::GetSphereDataSize(size_t SphereSteps)
{
    this->SphereSteps = SphereSteps;

    vertexCount = (SphereSteps + 1) * (SphereSteps + 1);
    indexCount = SphereSteps * SphereSteps * 6;

    sphereVertices.resize(vertexCount);
    indices.resize(indexCount);

    m_sphereIndexCount = (UINT)indexCount;
}

void Sphere::CreateSphere()
{
    UINT16* pIndices = indices.data();
    XMFLOAT3* pPos   = sphereVertices.data();
    
    for (size_t lat = 0; lat < SphereSteps + 1; lat++)
    {
        for (size_t lon = 0; lon < SphereSteps + 1; lon++)
        {
            int index = (int)(lat * (SphereSteps + 1) + lon);
            float lonAngle = 2.0f * (float)M_PI * lon / SphereSteps + (float)M_PI;
            float latAngle = -(float)M_PI / 2 + (float)M_PI * lat / SphereSteps;

            XMFLOAT3 r = XMFLOAT3{
                sinf(lonAngle) * cosf(latAngle),
                sinf(latAngle),
                cosf(lonAngle) * cosf(latAngle)
            };

            pPos[index] = r * 0.5f;
        }
    }

    for (size_t lat = 0; lat < SphereSteps; lat++)
    {
        for (size_t lon = 0; lon < SphereSteps; lon++)
        {
            size_t index = lat * SphereSteps * 6 + lon * 6;
            pIndices[index + 0] = (UINT16)(lat * (SphereSteps + 1) + lon + 0);
            pIndices[index + 2] = (UINT16)(lat * (SphereSteps + 1) + lon + 1);
            pIndices[index + 1] = (UINT16)(lat * (SphereSteps + 1) + SphereSteps + 1 + lon);
            pIndices[index + 3] = (UINT16)(lat * (SphereSteps + 1) + lon + 1);
            pIndices[index + 5] = (UINT16)(lat * (SphereSteps + 1) + SphereSteps + 1 + lon + 1);
            pIndices[index + 4] = (UINT16)(lat * (SphereSteps + 1) + SphereSteps + 1 + lon);
        }
    }
}

HRESULT Sphere::CreateVertexBuffer(ID3D11Device* m_pDevice)
{
    HRESULT result{};

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)(sphereVertices.size() * sizeof(XMFLOAT3));
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = sphereVertices.data();
    data.SysMemPitch = (UINT)(sphereVertices.size() * sizeof(XMFLOAT3));
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pSphereVertexBuffer);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        std::string name = "SphereVertexBuffers";

        result = m_pSphereVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT Sphere::CreateIndexBuffer(ID3D11Device* m_pDevice)
{
    HRESULT result{};

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)(indices.size() * sizeof(UINT16));
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = indices.data();
    data.SysMemPitch = (UINT)(indices.size() * sizeof(UINT16));
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pSphereIndexBuffer);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        std::string name = "SphereIndexBuffer";

        result = m_pSphereIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT Sphere::CreateGeometryBuffer(ID3D11Device* m_pDevice)
{
    HRESULT result{};

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(SphereGeomBuffer);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    SphereGeomBuffer geomBuffer;
    geomBuffer.m = DirectX::XMMatrixIdentity();
    geomBuffer.size.x = 2.0f;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &geomBuffer;
    data.SysMemPitch = sizeof(geomBuffer);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pSphereGeomBuffer);
    assert(SUCCEEDED(result));
  
    if (SUCCEEDED(result))
    {
        std::string name = "SphereGeomBuffer";

        result = m_pSphereGeomBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

void Sphere::CleanupSphere()
{
    if (m_pSphereVertexBuffer)
    {
        m_pSphereVertexBuffer->Release();
        m_pSphereVertexBuffer = nullptr;
    }

    if (m_pSphereGeomBuffer)
    {
        m_pSphereGeomBuffer->Release();
        m_pSphereGeomBuffer = nullptr;
    }

    if (m_pSphereIndexBuffer)
    {
        m_pSphereIndexBuffer->Release();
        m_pSphereIndexBuffer = nullptr;
    }
}


