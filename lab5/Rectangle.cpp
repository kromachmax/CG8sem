#include "Rectangle.h"

const D3D11_INPUT_ELEMENT_DESC RECTANGLE::Rectangle::InputDesc[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const VertexRect RECTANGLE::Rectangle::Vertices[] = {
    {0.0f, -0.75f, -0.75f, RGB(128, 0, 128)},
    {0.0f,  0.75f, -0.75f, RGB(128, 0, 128)},
    {0.0f,  0.75f,  0.75f, RGB(128, 0, 128)},
    {0.0f, -0.75f,  0.75f, RGB(128, 0, 128)}
};

const UINT16 RECTANGLE::Rectangle::Indices[] = {
    0, 1, 2,
    0, 2, 3
};

ID3D11Buffer* RECTANGLE::Rectangle::m_pRectangleVertexBuffer = nullptr;

ID3D11Buffer* RECTANGLE::Rectangle::m_pRectangleIndexBuffer = nullptr;

RECTANGLE::Rectangle::Rectangle()
    : m_pRectangleGeomBuffer(nullptr)
{
}

HRESULT RECTANGLE::Rectangle::CreateVertexBuffer(ID3D11Device* m_pDevice)
{
    HRESULT result;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)sizeof(Vertices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = Vertices;
    data.SysMemPitch = (UINT)sizeof(Vertices);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pRectangleVertexBuffer);
    assert(SUCCEEDED(result));
 
    if (SUCCEEDED(result))
    {
        std::string name = "RectVertexBuffer";

        result = m_pRectangleVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT RECTANGLE::Rectangle::CreateIndexBuffer(ID3D11Device* m_pDevice)
{
    HRESULT result;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)sizeof(Indices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = Indices;
    data.SysMemPitch = (UINT)sizeof(Indices);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pRectangleIndexBuffer);
    assert(SUCCEEDED(result));
 
    if (SUCCEEDED(result))
    {
        std::string name = "RectIndexBuffer";

        result = m_pRectangleIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT RECTANGLE::Rectangle::CreateGeometryBuffer(ID3D11Device* m_pDevice, const std::string& name, const RectGeomBuffer& geomBuffer)
{
    HRESULT result;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(RectGeomBuffer);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &geomBuffer;
    data.SysMemPitch = sizeof(geomBuffer);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pRectangleGeomBuffer);
    assert(SUCCEEDED(result));
 
    if (SUCCEEDED(result))
    {
        result = m_pRectangleIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

void RECTANGLE::Rectangle::CleanupRectangle()
{
    if (m_pRectangleGeomBuffer)
    {
        m_pRectangleGeomBuffer->Release();
        m_pRectangleGeomBuffer = nullptr;
    }

    if (m_pRectangleVertexBuffer)
    {
        m_pRectangleVertexBuffer->Release();
        m_pRectangleVertexBuffer = nullptr;
    }

    if (m_pRectangleIndexBuffer)
    {
        m_pRectangleIndexBuffer->Release();
        m_pRectangleIndexBuffer = nullptr;
    }
}
