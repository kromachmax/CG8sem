#pragma once

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

#include "framework.h"

#include "Sphere.h"

struct Vertex
{
    float x, y, z;
    COLORREF color;
};

struct TextureVertex
{
    float x, y, z;
    float u, v;
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
    XMFLOAT4 cameraPos;
};

struct Camera
{
    XMFLOAT3 poi;
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
        , m_prevUSec(0)
        , m_angle(0.0)
        , PressedKeys{ false }
        , m_pTexture(nullptr)
        , m_pTextureView(nullptr)
        , m_pSampler(nullptr)
        , m_pSpherePixelShader(nullptr)
        , m_pSphereVertexShader(nullptr)
        , m_pSphereInputLayout(nullptr)
        , m_pSphere(nullptr)
        , m_pCubemapTexture(nullptr)
        , m_pCubemapView(nullptr)
    {
    }

    bool InitDevice(HWND hWnd);
    void InitDebugLayer();

    void CleanupDevice();

    bool Render();
    bool Update();
    bool Resize(UINT width, UINT height);

    void SetPressedKeys(WPARAM pressedKey, bool flag);

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    HRESULT SetupBackBuffer();
    HRESULT InitScene();

    HRESULT CreateShader(const std::wstring& path, ShaderType shaderType,
        ID3D11DeviceChild** ppShader, ID3DBlob** ppCode = nullptr);

    void UpdateCamera(double deltaSec);

    HRESULT CreateVertexBuffer();
    HRESULT CreateIndexBuffer();
    HRESULT CreateGeomBuffer();
    HRESULT CreateSceneBuffer();
    HRESULT CreateSampler();

    HRESULT LoadTexture();

    HRESULT InitSphere();
    HRESULT InitCubemap();

    void RenderSphere();

private:

    ID3D11Device*        m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;

    IDXGISwapChain*         m_pSwapChain;
    ID3D11RenderTargetView* m_pBackBufferRTV;

    UINT m_width;
    UINT m_height;

    Camera m_camera{};

    size_t m_prevUSec;

    double m_angle;

    bool m_isMouseRotating = false;
    POINT m_lastMousePos{};
    float m_mouseSensitivity = 0.005f;

    static const float CameraRotationSpeed;
    static const float CameraMovingSpeed;
    static const float ModelRotationSpeed;

    bool PressedKeys[1024];

    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pIndexBuffer;
    ID3D11Buffer* m_pSceneBuffer;
    ID3D11Buffer* m_pGeomBuffer;

    ID3D11PixelShader*  m_pPixelShader;
    ID3D11VertexShader* m_pVertexShader;

    ID3D11InputLayout* m_pInputLayout;

    ID3D11Texture2D*            m_pTexture;
    ID3D11ShaderResourceView*   m_pTextureView;
    ID3D11SamplerState*         m_pSampler;


    ID3D11PixelShader*  m_pSpherePixelShader;
    ID3D11VertexShader* m_pSphereVertexShader;
    ID3D11InputLayout*  m_pSphereInputLayout;
    Sphere*             m_pSphere;

    ID3D11Texture2D* m_pCubemapTexture;
    ID3D11ShaderResourceView* m_pCubemapView;
};

