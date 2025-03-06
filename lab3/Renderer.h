#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <math.h>
#include <stdlib.h>

#include <corecrt_math_defines.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#include "framework.h"

struct Vertex
{
    float x, y, z;
    COLORREF color;
};

enum class ShaderType {
    Vertex,
    Pixel
};

struct GeomBuffer
{
    DirectX::XMMATRIX m;
};

struct SceneBuffer
{
    DirectX::XMMATRIX vp;
};

struct Point
{
    float x;
    float y;
    float z;
};

struct Camera
{
    Point poi;
    float r;
    float phi;
    float theta;
};

class Renderer
{
public:

    Renderer()
        : m_pDevice(nullptr)
        , m_pDeviceContext(nullptr)
        , m_pSwapChain(nullptr)
        , m_pBackBufferRTV(nullptr)
        , m_width(16)
        , m_height(16)
        , m_pPixelShader(nullptr)
        , m_pVertexShader(nullptr)
        , m_pInputLayout(nullptr)
        , m_pVertexBuffer(nullptr)
        , m_pIndexBuffer(nullptr)
        , m_pGeomBuffer(nullptr)
        , m_pSceneBuffer(nullptr)
        , m_pRasterizerState(nullptr)
        , m_prevUSec(0)
        , m_rotateModel(true)
        , m_angle(0.0)
        , m_camera()
    {
    }

    bool InitDevice(HWND hWnd);
    void InitDebugLayer();

    void CleanupDevice();

    bool Render();
    bool Update();
    bool Resize(UINT width, UINT height);

private:
    HRESULT SetupBackBuffer();
    HRESULT InitScene();

    HRESULT CreateShader(const std::wstring& path, ShaderType shaderType,
        ID3D11DeviceChild** ppShader, ID3DBlob** ppCode = nullptr);

private:
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pBackBufferRTV;

    UINT m_width;
    UINT m_height;

    Camera m_camera;

    size_t m_prevUSec;

    bool   m_rotateModel;
    double m_angle;

    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pIndexBuffer;
    ID3D11Buffer* m_pSceneBuffer;
    ID3D11Buffer* m_pGeomBuffer;

    ID3D11PixelShader* m_pPixelShader;
    ID3D11VertexShader* m_pVertexShader;
    ID3D11InputLayout* m_pInputLayout;

    ID3D11RasterizerState* m_pRasterizerState;
};

