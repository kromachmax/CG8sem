
#include "framework.h"
#include "Renderer.h"
#include "DDS.h"

const float Renderer::CameraRotationSpeed   = (float)M_PI * 2.0f;
const float Renderer::CameraMovingSpeed     = (float)10.0f;
const float Renderer::ModelRotationSpeed    = (float)M_PI / 2.0f;


bool Renderer::InitDevice(HWND hWnd)
{
    HRESULT result;

    IDXGIFactory* pFactory = nullptr;
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

    IDXGIAdapter* pSelectedAdapter = NULL;

    if (SUCCEEDED(result))
    {
        IDXGIAdapter* pAdapter = NULL;
        UINT adapterIdx = 0;

        while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter)))
        {
            DXGI_ADAPTER_DESC desc;
            pAdapter->GetDesc(&desc);

            if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0)
            {
                pSelectedAdapter = pAdapter;
                break;
            }

            pAdapter->Release();

            adapterIdx++;
        }
    }
    assert(pSelectedAdapter != NULL);


    D3D_FEATURE_LEVEL level;

    D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    if (SUCCEEDED(result))
    {

        UINT flags = 0;
#ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        result = D3D11CreateDevice(
            pSelectedAdapter,
            D3D_DRIVER_TYPE_UNKNOWN,
            NULL,
            flags,
            levels,
            1,
            D3D11_SDK_VERSION,
            &m_pDevice,
            &level,
            &m_pDeviceContext);

        assert(level == D3D_FEATURE_LEVEL_11_0);
        assert(SUCCEEDED(result));
    }

    InitDebugLayer();


    if (SUCCEEDED(result))
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };

        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = m_width;
        swapChainDesc.BufferDesc.Height = m_height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = true;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Flags = 0;

        result = pFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
        assert(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        m_camera.poi = XMFLOAT3{ 0,0,0 };
        m_camera.r = 5.0f;
        m_camera.phi = -(float)M_PI / 4;
        m_camera.theta = (float)M_PI / 4;
    }

    m_pScene = new SceneBuffer();

    if (SUCCEEDED(result))
    {
        m_pScene->lightCount.x = 1;
        m_pScene->lights[0].pos = XMFLOAT4{ 2.0, 1.0f, 0, 1 };
        m_pScene->lights[0].color = XMFLOAT4{ 1, 1, 0, 0};
        m_pScene->ambientColor = XMFLOAT4(0, 0, 0.2f, 0);
    }

    if (SUCCEEDED(result))
    {
        result = SetupBackBuffer();
    }

    if (SUCCEEDED(result))
    {
        result = InitScene();
    }

    pSelectedAdapter->Release();
    pFactory->Release();

    return SUCCEEDED(result);
}


void Renderer::InitDebugLayer()
{
    HRESULT hr;

    ID3D11Debug* debugDevice = nullptr;

    hr = m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugDevice);

    if (SUCCEEDED(hr) && debugDevice)
    {
        ID3D11InfoQueue* infoQueue = nullptr;

        hr = debugDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue);

        if (SUCCEEDED(hr) && infoQueue)
        {
            infoQueue->SetMuteDebugOutput(FALSE);


            D3D11_MESSAGE_SEVERITY severities[] = {
                D3D11_MESSAGE_SEVERITY_CORRUPTION,
                D3D11_MESSAGE_SEVERITY_ERROR,
                D3D11_MESSAGE_SEVERITY_WARNING
            };

            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.AllowList.NumSeverities = 3;
            filter.AllowList.pSeverityList = severities;

            infoQueue->PushStorageFilter(&filter);

            infoQueue->Release();
        }

        debugDevice->Release();
    }
}


void Renderer::CleanupDevice()
{
    if (m_pDeviceContext)
    {
        m_pDeviceContext->Release();
        m_pDeviceContext = nullptr;
    }

    if (m_pVertexBuffer)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }

    if (m_pIndexBuffer)
    {
        m_pIndexBuffer->Release();
        m_pIndexBuffer = nullptr;
    }

    if (m_pBackBufferRTV)
    {
        m_pBackBufferRTV->Release();
        m_pBackBufferRTV = nullptr;
    }

    if (m_pSwapChain)
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }

    if (m_pVertexShader)
    {
        m_pVertexShader->Release();
        m_pVertexShader = nullptr;
    }

    if (m_pPixelShader)
    {
        m_pPixelShader->Release();
        m_pPixelShader = nullptr;
    }

    if (m_pInputLayout)
    {
        m_pInputLayout->Release();
        m_pInputLayout = nullptr;
    }

    if (m_pGeomBuffer)
    {
        m_pGeomBuffer->Release();
        m_pGeomBuffer = nullptr;
    }

    if (m_pGeomBuffer2)
    {
        m_pGeomBuffer2->Release();
        m_pGeomBuffer2 = nullptr;
    }

    if (m_pSceneBuffer)
    {
        m_pSceneBuffer->Release();
        m_pSceneBuffer = nullptr;
    }

    if (m_pTexture)
    {
        m_pTexture->Release();
        m_pTexture = nullptr;
    }

    if (m_pTextureView)
    {
        m_pTextureView->Release();
        m_pTextureView = nullptr;
    }

    if (m_pSampler)
    {
        m_pSampler->Release();
        m_pSampler = nullptr;
    }

    if (m_pSpherePixelShader)
    {
        m_pSpherePixelShader->Release();
        m_pSpherePixelShader = nullptr;
    }

    if (m_pSphereVertexShader)
    {
        m_pSphereVertexShader->Release();
        m_pSphereVertexShader = nullptr;
    }

    if (m_pSphereInputLayout)
    {
        m_pSphereInputLayout->Release();
        m_pSphereInputLayout = nullptr;
    }

    if (m_pCubemapTexture)
    {
        m_pCubemapTexture->Release();
        m_pCubemapTexture = nullptr;
    }

    if (m_pCubemapView)
    {
        m_pCubemapView->Release();
        m_pCubemapView = nullptr;
    }

    if (m_pDepthBuffer)
    {
        m_pDepthBuffer->Release();
        m_pDepthBuffer = nullptr;
    }

    if (m_pDepthStencilView)
    {
        m_pDepthStencilView->Release();
        m_pDepthStencilView = nullptr;
    }

    if (m_pDepthState)
    {
        m_pDepthState->Release();
        m_pDepthState = nullptr;
    }

    if (m_pTransDepthState)
    {
        m_pTransDepthState->Release();
        m_pTransDepthState = nullptr;
    }

    if (m_pTransBlendState)
    {
        m_pTransBlendState->Release();
        m_pTransBlendState = nullptr;
    }

    if (m_pRectPixelShader)
    {
        m_pRectPixelShader->Release();
        m_pRectPixelShader = nullptr;
    }

    if (m_pRectVertexShader)
    {
        m_pRectVertexShader->Release();
        m_pRectVertexShader = nullptr;
    }

    if (m_pRectInputLayout)
    {
        m_pRectInputLayout->Release();
        m_pRectInputLayout = nullptr;
    }

    if (m_pRasterState)
    {
        m_pRasterState->Release();
        m_pRasterState = nullptr;
    }

    if (m_pLightInputLayout)
    {
        m_pLightInputLayout->Release();
        m_pLightInputLayout = nullptr;
    }

    if (m_pLightVertexShader)
    {
        m_pLightVertexShader->Release();
        m_pLightVertexShader = nullptr;
    }


    if (m_pLightPixelShader)
    {
        m_pLightPixelShader->Release();
        m_pLightPixelShader = nullptr;
    }

    if (m_pNoTransBlendState)
    {
        m_pNoTransBlendState->Release();
        m_pNoTransBlendState = nullptr;
    }

    if (m_pSphere != nullptr)
    {
        m_pSphere->CleanupSphere();
    }

    delete m_pSphere;

    if (m_pRect != nullptr)
    {
        m_pRect->CleanupRectangle();
    }

    delete m_pRect;

    if (m_pRect2 != nullptr)
    {
        m_pRect2->CleanupRectangle();
    }

    delete m_pRect2;

    for (int i = 0; i < m_pScene->lightCount.x; i++)
    {
        if (m_pLights[i] != nullptr)
        {
            m_pLights[i]->CleanupSphere();
        }

        delete m_pLights[i];
    }

    if (m_pScene)
    {
        delete m_pScene;
    }

#ifdef _DEBUG
    if (m_pDevice != nullptr)
    {
        ID3D11Debug* pDebug = nullptr;
        HRESULT result = m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&pDebug);
        assert(SUCCEEDED(result));

        if (pDebug != nullptr)
        {
            ULONG deviceRefCount = m_pDevice->AddRef();

            m_pDevice->Release();

            if (deviceRefCount != 3)
            {
                pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
            }

            pDebug->Release();
        }
    }
#endif

    if (m_pDevice)
    {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }

}


bool Renderer::Render()
{
    m_pDeviceContext->ClearState();

    ID3D11RenderTargetView* views[] = { m_pBackBufferRTV };
    m_pDeviceContext->OMSetRenderTargets(1, views, m_pDepthStencilView);

    static const FLOAT BackColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV, BackColor);
    m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)m_width;
    viewport.Height = (FLOAT)m_height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_pDeviceContext->RSSetViewports(1, &viewport);

    D3D11_RECT rect{};
    rect.left = 0;
    rect.top = 0;
    rect.right = m_width;
    rect.bottom = m_height;

    m_pDeviceContext->RSSetScissorRects(1, &rect);

    m_pDeviceContext->RSSetState(m_pRasterState);

    m_pDeviceContext->OMSetDepthStencilState(m_pDepthState, 0);

    m_pDeviceContext->OMSetBlendState(m_pTransBlendState, nullptr, 0xFFFFFFFF);

    ID3D11SamplerState* samplers[] = { m_pSampler };
    m_pDeviceContext->PSSetSamplers(0, 1, samplers);

    ID3D11ShaderResourceView* resources[] = { m_pTextureView };
    m_pDeviceContext->PSSetShaderResources(0, 1, resources);

    m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    ID3D11Buffer* vertexBuffers[] = { m_pVertexBuffer };

    UINT strides[] = { 44 };
    UINT offsets[] = { 0 };

    ID3D11Buffer* cbuffers[] = { m_pSceneBuffer, m_pGeomBuffer };

    m_pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
    m_pDeviceContext->IASetInputLayout(m_pInputLayout);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
    m_pDeviceContext->DrawIndexed(36, 0, 0);

    ID3D11Buffer* cbuffers2[] = { m_pGeomBuffer2 };
    m_pDeviceContext->VSSetConstantBuffers(1, 1, cbuffers2);
    m_pDeviceContext->PSSetConstantBuffers(1, 1, cbuffers2);
    m_pDeviceContext->DrawIndexed(36, 0, 0);

    for (int i = 0; i < m_pScene->lightCount.x; i++)
    {
        RenderLights(i);
    }

    RenderSphere();

    RenderRectangles();

    HRESULT result = m_pSwapChain->Present(0, 0);
    assert(SUCCEEDED(result));

    return SUCCEEDED(result);

}


bool Renderer::Update()
{
    size_t usec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    if (m_prevUSec == 0)
    {
        m_prevUSec = usec;
    }

    double deltaSec = (usec - m_prevUSec) / 1000000.0;

    m_angle = m_angle + deltaSec * ModelRotationSpeed;

    GeomBuffer geomBuffer;

    DirectX::XMMATRIX m = DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), -(float)m_angle);

    geomBuffer.m = m;

    m = DirectX::XMMatrixInverse(nullptr, m);
    m = DirectX::XMMatrixTranspose(m);
    geomBuffer.normalMatrix = m;
    geomBuffer.shine.x = 0.0f;

    m_pDeviceContext->UpdateSubresource(m_pGeomBuffer, 0, nullptr, &geomBuffer, 0, 0);

    m = DirectX::XMMatrixTranslation(2.0f, 0.0f, 0.0f);

    geomBuffer.m = m;

    m = DirectX::XMMatrixInverse(nullptr, m);
    m = DirectX::XMMatrixTranspose(m);
    geomBuffer.normalMatrix = m;
    geomBuffer.shine.x = 0.0f;

    m_pDeviceContext->UpdateSubresource(m_pGeomBuffer2, 0, nullptr, &geomBuffer, 0, 0);

    for (int i = 0; i < m_pScene->lightCount.x; i++)
    {
        SphereGeomBuffer geomBuffer;
        geomBuffer.m = DirectX::XMMatrixTranslation(m_pScene->lights[i].pos.x, m_pScene->lights[i].pos.y, m_pScene->lights[i].pos.z);
        geomBuffer.size = 1.0f;
        geomBuffer.color = m_pScene->lights[i].color;

        m_pLights[i]->UpdateGeomtryBuffer(m_pDeviceContext, &geomBuffer);
    }

    UpdateCamera(deltaSec);

    m_prevUSec = usec;

    DirectX::XMMATRIX v;
    XMFLOAT4 cameraPos;
    {
        float posX = m_camera.poi.x + cosf(m_camera.theta) * cosf(m_camera.phi) * m_camera.r;
        float posY = m_camera.poi.y + sinf(m_camera.theta) * m_camera.r;
        float posZ = m_camera.poi.z + cosf(m_camera.theta) * sinf(m_camera.phi) * m_camera.r;

        float upTheta = m_camera.theta + (float)M_PI / 2;

        float upX = cosf(upTheta) * cosf(m_camera.phi);
        float upY = sinf(upTheta);
        float upZ = cosf(upTheta) * sinf(m_camera.phi);

        v = DirectX::XMMatrixLookAtLH(
            DirectX::XMVectorSet(posX, posY, posZ, 0.0f),
            DirectX::XMVectorSet(m_camera.poi.x, m_camera.poi.y, m_camera.poi.z, 0.0f),
            DirectX::XMVectorSet(upX, upY, upZ, 0.0f)
        );


        cameraPos = { posX, posY, posZ };
    }

    float f = 100.0f;
    float n = 0.1f;
    float fov = (float)M_PI / 3;
    float c = 1.0f / tanf(fov / 2);
    float aspectRatio = (float)m_height / m_width;
    DirectX::XMMATRIX p = DirectX::XMMatrixPerspectiveLH(tanf(fov / 2) * 2 * n, tanf(fov / 2) * 2 * n * aspectRatio, n, f);

    D3D11_MAPPED_SUBRESOURCE subresource;
    HRESULT result = m_pDeviceContext->Map(m_pSceneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        SceneBuffer& sceneBuffer = *reinterpret_cast<SceneBuffer*>(subresource.pData);

        sceneBuffer.vp = DirectX::XMMatrixMultiply(v, p);
        sceneBuffer.cameraPos = cameraPos;

        m_pDeviceContext->Unmap(m_pSceneBuffer, 0);
    }

    return SUCCEEDED(result);
}


bool Renderer::Resize(UINT width, UINT height)
{
    if (width != m_width || height != m_height)
    {
        m_pBackBufferRTV->Release();

        if (m_pDepthBuffer)
    {
        m_pDepthBuffer->Release();
        m_pDepthBuffer = nullptr;
    }

    if (m_pDepthStencilView)
    {
        m_pDepthStencilView->Release();
        m_pDepthStencilView = nullptr;
    }

        HRESULT result = m_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

        assert(SUCCEEDED(result));

        if (SUCCEEDED(result))
        {
            m_width = width;
            m_height = height;

            result = SetupBackBuffer();
        }

        return SUCCEEDED(result);
    }

    return true;
}


HRESULT Renderer::SetupBackBuffer()
{
    ID3D11Texture2D* pBackBuffer = NULL;

    HRESULT result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pBackBufferRTV);
        assert(SUCCEEDED(result));

        pBackBuffer->Release();
    }

    if (SUCCEEDED(result))
    {
        D3D11_TEXTURE2D_DESC desc{};

        desc.Format             = DXGI_FORMAT_D32_FLOAT;
        desc.ArraySize          = 1;
        desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.Height             = m_height;
        desc.Width              = m_width;
        desc.MipLevels          = 1;

        result = m_pDevice->CreateTexture2D(&desc, nullptr, &m_pDepthBuffer);
     
        if (SUCCEEDED(result))
        {
            std::string name = "DepthBuffer";

            result = m_pDepthBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }

    }
    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateDepthStencilView(m_pDepthBuffer, nullptr, &m_pDepthStencilView);
    
        if (SUCCEEDED(result))
        {
            std::string name = "DepthBufferView";

            result = m_pDepthStencilView->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }

    }

    assert(SUCCEEDED(result));

    return result;
}


HRESULT Renderer::InitScene()
{
    HRESULT result {};

    result = CreateVertexBuffer();

    if (SUCCEEDED(result))
    {
        result = CreateIndexBuffer();
    }

    static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    ID3DBlob* pVertexShaderCode = nullptr;

    if (SUCCEEDED(result))
    {
        result = CreateShader(L"VertexShader.hlsl", ShaderType::Vertex, (ID3D11DeviceChild**)&m_pVertexShader, &pVertexShaderCode);
    }

    if (SUCCEEDED(result))
    {
        result = CreateShader(L"PixelShader.hlsl", ShaderType::Pixel, (ID3D11DeviceChild**)&m_pPixelShader);
    }

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(InputDesc, 2, pVertexShaderCode->GetBufferPointer(), pVertexShaderCode->GetBufferSize(), &m_pInputLayout);

        if (SUCCEEDED(result))
        {
            std::string name = "InputLayout";

            result = m_pInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }
    }

    pVertexShaderCode->Release();

    if (SUCCEEDED(result))
    {
        result = CreateRasterizerState();
    }

    if (SUCCEEDED(result))
    {
        result = CreateGeomBuffer();
    }

    if (SUCCEEDED(result))
    {
        result = CreateSceneBuffer();
    }

    if (SUCCEEDED(result))
    {
        result = CreateBlendState();
    }

    if (SUCCEEDED(result))
    {
        result = CreateDepthState();
    }

    if (SUCCEEDED(result))
    {
        result = CreateTPDepthState();
    }

    if (SUCCEEDED(result))
    {
        result = LoadTexture();
    }

    if (SUCCEEDED(result))
    {
        result = CreateSampler();
    }

    if (SUCCEEDED(result))
    {
        result = InitSphere();
        assert(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        result = InitRect();
        assert(SUCCEEDED(result));
    }


    if (SUCCEEDED(result))
    {
        result = InitCubemap();
        assert(SUCCEEDED(result));
    }

    for (int i = 0; i < m_pScene->lightCount.x; ++i)
    {
        if (SUCCEEDED(result))
        {
            result = InitLights(i);
        }
    }

    return result;
}

class D3DInclude : public ID3DInclude
{
    STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
    {
        FILE* pFile = nullptr;
        fopen_s(&pFile, pFileName, "rb");
        assert(pFile != nullptr);
        if (pFile == nullptr)
        {
            return E_FAIL;
        }

        fseek(pFile, 0, SEEK_END);
        long long size = _ftelli64(pFile);
        fseek(pFile, 0, SEEK_SET);

        VOID* pData = malloc(size);
        if (pData == nullptr)
        {
            fclose(pFile);
            return E_FAIL;
        }

        size_t rd = fread(pData, 1, size, pFile);
        assert(rd == (size_t)size);

        if (rd != (size_t)size)
        {
            fclose(pFile);
            free(pData);
            return E_FAIL;
        }

        *ppData = pData;
        *pBytes = (UINT)size;

        return S_OK;
    }
    STDMETHOD(Close)(THIS_ LPCVOID pData)
    {
        free(const_cast<void*>(pData));
        return S_OK;
    }
};


HRESULT Renderer::CreateShader(const std::wstring& path, ShaderType shaderType, ID3D11DeviceChild** ppShader, ID3DBlob** ppCode)
{

    FILE* pFile = nullptr;
    _wfopen_s(&pFile, path.c_str(), L"rb");

    if (pFile == nullptr)
    {
        return E_FAIL;
    }


    fseek(pFile, 0, SEEK_END);
    long long size = _ftelli64(pFile);
    fseek(pFile, 0, SEEK_SET);


    std::vector<char> data(size + 1);
    size_t rd = fread(data.data(), 1, size, pFile);
    assert(rd == (size_t)size);
    fclose(pFile);


    std::string entryPoint;
    std::string platform;

    switch (shaderType)
    {
    case ShaderType::Vertex:
    {
        entryPoint  = "VS";
        platform    = "vs_5_0";
        break;
    }

    case ShaderType::Pixel:
    {
        entryPoint  = "PS";
        platform    = "ps_5_0";
        break;
    }

    default:
    {
        assert(false && "Unknown shader type");
        return E_INVALIDARG;
    }
    }


    UINT flags1 = 0;
#ifdef _DEBUG
    flags1 |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif


    D3DInclude includeHandler;

    ID3DBlob* pCode     = nullptr;
    ID3DBlob* pErrMsg   = nullptr;
    HRESULT result      = D3DCompile(data.data(), data.size(), nullptr,
                                     nullptr, &includeHandler, entryPoint.c_str(), platform.c_str(),
                                     flags1, 0, &pCode, &pErrMsg);


    if (!SUCCEEDED(result) && pErrMsg != nullptr)
    {
        OutputDebugStringA((const char*)pErrMsg->GetBufferPointer());
    }

    assert(SUCCEEDED(result));

    if (pErrMsg != nullptr)
    {
        pErrMsg->Release();
    }


    if (SUCCEEDED(result))
    {
        if (shaderType == ShaderType::Vertex)
        {
            ID3D11VertexShader* pVertexShader = nullptr;

            result = m_pDevice->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pVertexShader);

            if (SUCCEEDED(result))
            {
                *ppShader = pVertexShader;
            }
        }
        else if (shaderType == ShaderType::Pixel)
        {
            ID3D11PixelShader* pPixelShader = nullptr;

            result = m_pDevice->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pPixelShader);

            if (SUCCEEDED(result))
            {
                *ppShader = pPixelShader;
            }
        }
    }

    if (ppCode)
    {
        *ppCode = pCode;
    }
    else
    {
        pCode->Release();
    }

    return result;
}


void Renderer::UpdateCamera(double deltaSec)
{

    float cameraSpeed = CameraMovingSpeed * (float)deltaSec;


    float dirX = cosf(m_camera.theta) * cosf(m_camera.phi);
    float dirY = sinf(m_camera.theta);
    float dirZ = cosf(m_camera.theta) * sinf(m_camera.phi);


    float upTheta = m_camera.theta + (float)M_PI / 2;
    float upX = cosf(upTheta) * cosf(m_camera.phi);
    float upY = sinf(upTheta);
    float upZ = cosf(upTheta) * sinf(m_camera.phi);


    float rightX = dirY * upZ - dirZ * upY;
    float rightY = dirZ * upX - dirX * upZ;
    float rightZ = dirX * upY - dirY * upX;


    float rightLen = sqrtf(rightX * rightX + rightY * rightY + rightZ * rightZ);


    if (rightLen > 0.0f) {
        rightX /= rightLen;
        rightY /= rightLen;
        rightZ /= rightLen;
    }

    // Движение вперед (W)
    if (PressedKeys['W'])
    {
        m_camera.r -= cameraSpeed;
        if (m_camera.r < 0.5f)
        {
            m_camera.r = 0.5f;
        }

        PressedKeys['W'] = false;
    }

    // Движение назад (S)
    if (PressedKeys['S'])
    {
        m_camera.r += cameraSpeed;
        PressedKeys['S'] = false;
    }


    if (PressedKeys['D'])
    {
        m_camera.poi.x += rightX * cameraSpeed;
        m_camera.poi.y += rightY * cameraSpeed;
        m_camera.poi.z += rightZ * cameraSpeed;

        PressedKeys['D'] = false;
    }


    if (PressedKeys['A'])
    {
        m_camera.poi.x -= rightX * cameraSpeed;
        m_camera.poi.y -= rightY * cameraSpeed;
        m_camera.poi.z -= rightZ * cameraSpeed;


        PressedKeys['A'] = false;
    }


    if (PressedKeys[VK_SPACE])
    {
        m_camera.poi.x += upX * cameraSpeed;
        m_camera.poi.y += upY * cameraSpeed;
        m_camera.poi.z += upZ * cameraSpeed;

        PressedKeys[VK_SPACE] = false;
    }


    if (PressedKeys[VK_CONTROL])
    {
        m_camera.poi.x -= upX * cameraSpeed;
        m_camera.poi.y -= upY * cameraSpeed;
        m_camera.poi.z -= upZ * cameraSpeed;

        PressedKeys[VK_CONTROL] = false;
    }
}



HRESULT Renderer::CreateVertexBuffer()
{
    HRESULT result;

    static const TextureNormalVertex Vertices[24] = {

       {XMFLOAT3{-0.5, -0.5,  0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, -1, 0}, 0, 1},
       {XMFLOAT3{ 0.5, -0.5,  0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, -1, 0}, 1, 1},
       {XMFLOAT3{ 0.5, -0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, -1, 0}, 1, 0},
       {XMFLOAT3{-0.5, -0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, -1, 0}, 0, 0},
       

       {XMFLOAT3{-0.5,  0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 1, 0}, 0, 1},
       {XMFLOAT3{-0.5,  0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 1, 0}, 1, 1},
       {XMFLOAT3{ 0.5,  0.5,  0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 1, 0}, 1, 0},
       {XMFLOAT3{-0.5,  0.5,  0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 1, 0}, 0, 0},
      
       {XMFLOAT3{ 0.5, -0.5, -0.5}, XMFLOAT3{0, 0, 1}, XMFLOAT3{1, 0, 0}, 0, 1},
       {XMFLOAT3{ 0.5, -0.5,  0.5}, XMFLOAT3{0, 0, 1}, XMFLOAT3{1, 0, 0}, 1, 1},
       {XMFLOAT3{ 0.5,  0.5,  0.5}, XMFLOAT3{0, 0, 1}, XMFLOAT3{1, 0, 0}, 1, 0},
       {XMFLOAT3{ 0.5,  0.5, -0.5}, XMFLOAT3{0, 0, 1}, XMFLOAT3{1, 0, 0}, 0, 0},
     
       {XMFLOAT3{-0.5, -0.5,  0.5}, XMFLOAT3{0, 0, -1}, XMFLOAT3{-1, 0, 0}, 0, 1},
       {XMFLOAT3{-0.5, -0.5, -0.5}, XMFLOAT3{0, 0, -1}, XMFLOAT3{-1, 0, 0}, 1, 1},
       {XMFLOAT3{-0.5,  0.5, -0.5}, XMFLOAT3{0, 0, -1}, XMFLOAT3{-1, 0, 0}, 1, 0},
       {XMFLOAT3{-0.5,  0.5,  0.5}, XMFLOAT3{0, 0, -1}, XMFLOAT3{-1, 0, 0}, 0, 0},
    
       {XMFLOAT3{ 0.5, -0.5,  0.5}, XMFLOAT3{-1, 0, 0}, XMFLOAT3{0, 0, 1}, 0, 1},
       {XMFLOAT3{-0.5, -0.5,  0.5}, XMFLOAT3{-1, 0, 0}, XMFLOAT3{0, 0, 1}, 1, 1},
       {XMFLOAT3{-0.5,  0.5,  0.5}, XMFLOAT3{-1, 0, 0}, XMFLOAT3{0, 0, 1}, 1, 0},
       {XMFLOAT3{ 0.5,  0.5,  0.5}, XMFLOAT3{-1, 0, 0}, XMFLOAT3{0, 0, 1}, 0, 0},
      
       {XMFLOAT3{-0.5, -0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 0, -1}, 0, 1},
       {XMFLOAT3{ 0.5, -0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 0, -1}, 1, 1},
       {XMFLOAT3{ 0.5,  0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 0, -1}, 1, 0},
       {XMFLOAT3{-0.5,  0.5, -0.5}, XMFLOAT3{1, 0, 0}, XMFLOAT3{0, 0, -1}, 0, 0}

    };

    D3D11_BUFFER_DESC desc{};

    desc.ByteWidth = sizeof(Vertices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data{};

    data.pSysMem = &Vertices;
    data.SysMemPitch = sizeof(Vertices);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pVertexBuffer);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        std::string name = "VertexBuffer";

        result = m_pVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}


HRESULT Renderer::CreateIndexBuffer()
{
    HRESULT result;

    static const UINT16 Indices[36] = {
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
    };

    D3D11_BUFFER_DESC desc{};
    D3D11_SUBRESOURCE_DATA data{};

    desc.ByteWidth = sizeof(Indices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    data.pSysMem = &Indices;
    data.SysMemPitch = sizeof(Indices);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pIndexBuffer);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        std::string name = "IndexBuffer";

        result = m_pIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT Renderer::CreateGeomBuffer()
{
    HRESULT result{};

    D3D11_BUFFER_DESC desc = {};

    desc.ByteWidth = sizeof(GeomBuffer);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    GeomBuffer geomBuffer;
    geomBuffer.m = DirectX::XMMatrixIdentity();

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &geomBuffer;
    data.SysMemPitch = sizeof(geomBuffer);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pGeomBuffer);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        std::string name = "Geombuffer";

        result = m_pGeomBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pGeomBuffer2);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        std::string name = "m_pGeomBuffer2";

        result = m_pGeomBuffer2->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}


HRESULT Renderer::CreateSceneBuffer()
{
    HRESULT result{};

    D3D11_BUFFER_DESC desc = {};

    desc.ByteWidth = sizeof(SceneBuffer);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    result = m_pDevice->CreateBuffer(&desc, nullptr, &m_pSceneBuffer);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {
        std::string name = "SceneBuffer";

        result = m_pSceneBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT Renderer::CreateSampler()
{
    HRESULT result;

    D3D11_SAMPLER_DESC desc = {};

    desc.Filter = D3D11_FILTER_ANISOTROPIC;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.MinLOD = -FLT_MAX;
    desc.MaxLOD = FLT_MAX;
    desc.MipLODBias = 0.0f;
    desc.MaxAnisotropy = 16;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    result = m_pDevice->CreateSamplerState(&desc, &m_pSampler);
    assert(SUCCEEDED(result));
    
    return result;
}

HRESULT Renderer::CreateDepthState()
{
    HRESULT result{};

    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = TRUE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable = FALSE;

    result = m_pDevice->CreateDepthStencilState(&desc, &m_pDepthState);

    if (SUCCEEDED(result))
    {
        std::string name = "DepthState";

        result = m_pDepthState->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT Renderer::CreateTPDepthState()
{
    HRESULT result;

    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = TRUE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable = FALSE;

    result = m_pDevice->CreateDepthStencilState(&desc, &m_pTransDepthState);

    if (SUCCEEDED(result))
    {
        std::string name = "TransDepthState";

        result = m_pTransDepthState->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT Renderer::CreateBlendState()
{
    HRESULT result;

    D3D11_BLEND_DESC desc = {};
    desc.AlphaToCoverageEnable = false;
    desc.IndependentBlendEnable = false;
    desc.RenderTarget[0].BlendEnable = true;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;

    result = m_pDevice->CreateBlendState(&desc, &m_pTransBlendState);

    if (SUCCEEDED(result))
    {
        std::string name = "TransBlendState";

        result = m_pTransBlendState->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    if (SUCCEEDED(result))
    {
        desc.RenderTarget[0].BlendEnable = FALSE;
        result = m_pDevice->CreateBlendState(&desc, &m_pNoTransBlendState);
    }
    if (SUCCEEDED(result))
    {
        std::string name = "NoTransBlendState";

        result = m_pNoTransBlendState->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}

HRESULT Renderer::CreateRasterizerState()
{
    HRESULT result{};

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;

    result = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterState);

    if (SUCCEEDED(result))
    {
        std::string name = "RasterState";

        result = m_pRasterState->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)name.length(), name.c_str());
    }

    return result;
}


HRESULT Renderer::LoadTexture()
{
    DXGI_FORMAT textureFmt;
    HRESULT result;

    const std::wstring TextureName = L"../textures/metal.dds";

    TextureDesc textureDesc;
    bool ddsRes = LoadDDS(TextureName.c_str(), textureDesc);

    textureFmt = textureDesc.fmt;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Format = textureDesc.fmt;
    desc.ArraySize = 1;
    desc.MipLevels = textureDesc.mipmapsCount;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Height = textureDesc.height;
    desc.Width = textureDesc.width;

    UINT32 blockWidth = DivUp(desc.Width, 4u);
    UINT32 blockHeight = DivUp(desc.Height, 4u);
    UINT32 pitch = blockWidth * GetBytesPerBlock(desc.Format);

    const char* pSrcData = reinterpret_cast<const char*>(textureDesc.pData);

    std::vector<D3D11_SUBRESOURCE_DATA> data;
    data.resize(desc.MipLevels);

    for (UINT32 i = 0; i < desc.MipLevels; i++)
    {
        data[i].pSysMem = pSrcData;
        data[i].SysMemPitch = pitch;
        data[i].SysMemSlicePitch = 0;

        pSrcData += pitch * blockHeight;
        blockHeight = std::max(1u, blockHeight / 2);
        blockWidth = std::max(1u, blockWidth / 2);
        pitch = blockWidth * GetBytesPerBlock(desc.Format);
    }

    result = m_pDevice->CreateTexture2D(&desc, data.data(), &m_pTexture);
    assert(SUCCEEDED(result));

    if (SUCCEEDED(result))
    {

        result = m_pTexture->SetPrivateData(WKPDID_D3DDebugObjectName,
            (UINT)TextureName.length(), TextureName.c_str());
    }

    free(textureDesc.pData);
    

    if (SUCCEEDED(result))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};

        desc.Format = textureFmt;
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = textureDesc.mipmapsCount;
        desc.Texture2D.MostDetailedMip = 0;

        result = m_pDevice->CreateShaderResourceView(m_pTexture, &desc, &m_pTextureView);
        assert(SUCCEEDED(result));
    }

    return result;
}


HRESULT Renderer::InitSphere()
{
    static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    HRESULT result = S_OK;
    static const size_t SphereSteps = 32;

    m_pSphere = new Sphere();

    m_pSphere->GetSphereDataSize(SphereSteps);
    m_pSphere->CreateSphere();

    if (SUCCEEDED(result))
    {
        m_pSphere->CreateVertexBuffer(m_pDevice);
    }

    if (SUCCEEDED(result))
    {
        m_pSphere->CreateIndexBuffer(m_pDevice);
    }

    ID3DBlob* pSphereVertexShaderCode = nullptr;
  
    if (SUCCEEDED(result))
    {
        result = CreateShader(L"VertexSphereShader.hlsl", ShaderType::Vertex, (ID3D11DeviceChild**)&m_pSphereVertexShader, &pSphereVertexShaderCode);
    }

    if (SUCCEEDED(result))
    {
        result = CreateShader(L"PixelSphereShader.hlsl", ShaderType::Pixel, (ID3D11DeviceChild**)&m_pSpherePixelShader);
    }

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(InputDesc, 1, pSphereVertexShaderCode->GetBufferPointer(), pSphereVertexShaderCode->GetBufferSize(), &m_pSphereInputLayout);
  
        if (SUCCEEDED(result))
        {
            std::string name = "SphereInputLayout";

            result = m_pSphereInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }
    }

    if (pSphereVertexShaderCode)
    {
        pSphereVertexShaderCode->Release();
        pSphereVertexShaderCode = nullptr;
    }

    if (SUCCEEDED(result))
    {
        m_pSphere->CreateGeometryBuffer(m_pDevice, {});
    }

    return result;
}



HRESULT Renderer::InitLights(int i)
{
    static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    HRESULT result = S_OK;
    static const size_t SphereSteps = 128;

    m_pLights[i] = new Sphere();

    m_pLights[i]->GetSphereDataSize(SphereSteps);
    m_pLights[i]->CreateSphere();
    m_pLights[i]->Scale();

    if (SUCCEEDED(result))
    {
        m_pLights[i]->CreateVertexBuffer(m_pDevice);
    }

    if (SUCCEEDED(result))
    {
        m_pLights[i]->CreateIndexBuffer(m_pDevice);
    }

    ID3DBlob* pLightVertexShaderCode = nullptr;

    if (SUCCEEDED(result))
    {
        result = CreateShader(L"VertexLightShader.hlsl", ShaderType::Vertex, (ID3D11DeviceChild**)&m_pLightVertexShader, &pLightVertexShaderCode);
    }

    if (SUCCEEDED(result))
    {
        result = CreateShader(L"PixelLightShader.hlsl", ShaderType::Pixel, (ID3D11DeviceChild**)&m_pLightPixelShader);
    }

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(InputDesc, 1, pLightVertexShaderCode->GetBufferPointer(), pLightVertexShaderCode->GetBufferSize(), &m_pLightInputLayout);

        if (SUCCEEDED(result))
        {
            std::string name = "LightInputLayout";

            result = m_pLightInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }
    }

    if (pLightVertexShaderCode)
    {
        pLightVertexShaderCode->Release();
        pLightVertexShaderCode = nullptr;
    }

    if (SUCCEEDED(result))
    {
        m_pLights[i]->CreateGeometryBuffer(m_pDevice, m_pScene->lights[i].color);
    }

    return result;
}


HRESULT Renderer::InitRect()
{
    HRESULT result = S_OK;

    m_pRect = new RECTANGLE::Rectangle();
    m_pRect2 = new RECTANGLE::Rectangle();

    if (SUCCEEDED(result))
    {
        result = RECTANGLE::Rectangle::CreateVertexBuffer(m_pDevice);
    }

    if (SUCCEEDED(result))
    {
        result = RECTANGLE::Rectangle::CreateIndexBuffer(m_pDevice);
    }


    ID3DBlob* pRectangleVertexShaderCode = nullptr;

    if (SUCCEEDED(result))
    {
        result = CreateShader(L"VertexRectangleShader.hlsl", ShaderType::Vertex, (ID3D11DeviceChild**)&m_pRectVertexShader, &pRectangleVertexShaderCode);
    }

    if (SUCCEEDED(result))
    {
        result = CreateShader(L"PixelRectangleShader.hlsl", ShaderType::Pixel, (ID3D11DeviceChild**)&m_pRectPixelShader);
    }

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(m_pRect->InputDesc, 2, pRectangleVertexShaderCode->GetBufferPointer(), 
                                                pRectangleVertexShaderCode->GetBufferSize(), &m_pRectInputLayout);

        if (SUCCEEDED(result))
        {
            std::string name = "RectangleInputLayout";

            result = m_pRectInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }
    }

    if (pRectangleVertexShaderCode)
    {
        pRectangleVertexShaderCode->Release();
        pRectangleVertexShaderCode = nullptr;
    }

    if (SUCCEEDED(result))
    {
        RECTANGLE::RectGeomBuffer geomBuffer;
        geomBuffer.m = DirectX::XMMatrixTranslation(1.0f, 0, 0);
        geomBuffer.color = XMFLOAT4{ 0.5f, 0, 0.5f, 1.0f };
        HRESULT hr1 = m_pRect->CreateGeometryBuffer(m_pDevice, "RectGeomBuffer", geomBuffer);

        geomBuffer.m = DirectX::XMMatrixTranslation(1.2f, 0, 0);
        geomBuffer.color = XMFLOAT4{ 0.0f, 0.5f, 0.0f, 1.0f };
        HRESULT hr2 = m_pRect2->CreateGeometryBuffer(m_pDevice, "RectGeomBuffer2", geomBuffer);

        result = SUCCEEDED(hr1) && SUCCEEDED(hr2) ? S_OK : E_FAIL;
    }

    return result;
}


HRESULT Renderer::InitCubemap()
{
    HRESULT result = S_OK;

    DXGI_FORMAT textureFmt;

    if (SUCCEEDED(result))
    {
        const std::wstring TextureNames[6] =
        {
            L"../textures/px.dds", L"../textures/nx.dds",
            L"../textures/py.dds", L"../textures/ny.dds",
            L"../textures/pz.dds", L"../textures/nz.dds"
        };

        TextureDesc texDescs[6];
        bool ddsRes = true;

        for (int i = 0; i < 6 && ddsRes; i++)
        {
            ddsRes = LoadDDS(TextureNames[i].c_str(), texDescs[i], true);
        }

        textureFmt = texDescs[0].fmt;

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Format = textureFmt;
        desc.ArraySize = 6;
        desc.MipLevels = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Height = texDescs[0].height;
        desc.Width = texDescs[0].width;

        UINT32 blockWidth = DivUp(desc.Width, 4u);
        UINT32 blockHeight = DivUp(desc.Height, 4u);
        UINT32 pitch = blockWidth * GetBytesPerBlock(desc.Format);

        D3D11_SUBRESOURCE_DATA data[6];

        for (int i = 0; i < 6; i++)
        {
            data[i].pSysMem = texDescs[i].pData;
            data[i].SysMemPitch = pitch;
            data[i].SysMemSlicePitch = 0;
        }

        result = m_pDevice->CreateTexture2D(&desc, data, &m_pCubemapTexture);
        assert(SUCCEEDED(result));
      
        if (SUCCEEDED(result))
        {
            std::string name = "CubemapTexture";

            result = m_pCubemapTexture->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }

        for (int i = 0; i < 6; i++)
        {
            if (texDescs[i].pData)
            {
                free(texDescs[i].pData);
            }
        }
    }


    if (SUCCEEDED(result))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format = textureFmt;
        desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
        desc.TextureCube.MipLevels = 1;
        desc.TextureCube.MostDetailedMip = 0;

        result = m_pDevice->CreateShaderResourceView(m_pCubemapTexture, &desc, &m_pCubemapView);
     
        if (SUCCEEDED(result))
        {
            std::string name = "CubemapView";

            result = m_pCubemapView->SetPrivateData(WKPDID_D3DDebugObjectName,
                (UINT)name.length(), name.c_str());
        }
    }

    return result;
}


void Renderer::RenderSphere()
{
    ID3D11SamplerState* samplers[] = { m_pSampler };
    m_pDeviceContext->PSSetSamplers(0, 1, samplers);

    ID3D11ShaderResourceView* resources[] = { m_pCubemapView };
    m_pDeviceContext->PSSetShaderResources(0, 1, resources);

    m_pDeviceContext->IASetIndexBuffer(m_pSphere->m_pSphereIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    ID3D11Buffer* vertexBuffers[] = { m_pSphere->m_pSphereVertexBuffer };
    UINT strides[] = { 12 };
    UINT offsets[] = { 0 };
    ID3D11Buffer* cbuffers[] = { m_pSceneBuffer, m_pSphere->m_pSphereGeomBuffer };
    ID3D11Buffer* ps_cbuffers[] = { m_pSceneBuffer };


    m_pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    m_pDeviceContext->IASetInputLayout(m_pSphereInputLayout);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->VSSetShader(m_pSphereVertexShader, nullptr, 0);
    m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
    m_pDeviceContext->PSSetShader(m_pSpherePixelShader, nullptr, 0);
    m_pDeviceContext->PSSetConstantBuffers(0, 1, ps_cbuffers);
    m_pDeviceContext->DrawIndexed(m_pSphere->m_sphereIndexCount, 0, 0);
}

void Renderer::RenderLights(int i)
{

    m_pDeviceContext->OMSetDepthStencilState(m_pDepthState, 0);
    m_pDeviceContext->OMSetBlendState(m_pNoTransBlendState, nullptr, 0xFFFFFFFF);

    ID3D11Buffer* vertexBuffers[] = { m_pLights[i]->m_pSphereVertexBuffer };
    UINT strides[] = { 12 };
    UINT offsets[] = { 0 };
    ID3D11Buffer* cbuffers[] = { m_pSceneBuffer, m_pLights[i]->m_pSphereGeomBuffer };


    m_pDeviceContext->IASetIndexBuffer(m_pLights[i]->m_pSphereIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    m_pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    m_pDeviceContext->IASetInputLayout(m_pLightInputLayout);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->VSSetShader(m_pLightVertexShader, nullptr, 0);
    m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
    m_pDeviceContext->PSSetShader(m_pLightPixelShader, nullptr, 0);
    m_pDeviceContext->PSSetConstantBuffers(0, 2, cbuffers);
    m_pDeviceContext->DrawIndexed(m_pLights[i]->m_sphereIndexCount, 0, 0);
}

void Renderer::RenderRectangles()
{
    m_pDeviceContext->OMSetDepthStencilState(m_pDepthState, 0);
    m_pDeviceContext->OMSetBlendState(m_pTransBlendState, nullptr, 0xFFFFFFFF);

    ID3D11Buffer* vertexBuffers[] = { RECTANGLE::Rectangle::GetVertexBuffer()};
    UINT strides[] = { 16 };
    UINT offsets[] = { 0 };
    ID3D11Buffer* cbuffers[] = { m_pSceneBuffer, nullptr };

    m_pDeviceContext->IASetIndexBuffer(RECTANGLE::Rectangle::GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);
    m_pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    m_pDeviceContext->IASetInputLayout(m_pRectInputLayout);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->VSSetShader(m_pRectVertexShader, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pRectPixelShader, nullptr, 0);

    XMFLOAT3 cameraPos;
    XMFLOAT3 rectPos;
    XMFLOAT3 rect2Pos;
    
    float posX = m_camera.poi.x + cosf(m_camera.theta) * cosf(m_camera.phi) * m_camera.r;
    float posY = m_camera.poi.y + sinf(m_camera.theta) * m_camera.r;
    float posZ = m_camera.poi.z + cosf(m_camera.theta) * sinf(m_camera.phi) * m_camera.r;

    cameraPos = { posX, posY, posZ };
    rectPos = m_pRect->GetCenterCoordinate();
    rect2Pos = m_pRect2->GetCenterCoordinate();

    if ((cameraPos - rectPos).LengthSquared() < (cameraPos - rect2Pos).LengthSquared())
    {
        cbuffers[1] = m_pRect2->GetGeomBuffer();

        m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->PSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->DrawIndexed(6, 0, 0);

        cbuffers[1] = m_pRect->GetGeomBuffer();

        m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->PSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->DrawIndexed(6, 0, 0);
    }
    else
    {
        cbuffers[1] = m_pRect->GetGeomBuffer();

        m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->PSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->DrawIndexed(6, 0, 0);

        cbuffers[1] = m_pRect2->GetGeomBuffer();

        m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->PSSetConstantBuffers(0, 2, cbuffers);
        m_pDeviceContext->DrawIndexed(6, 0, 0);
    }

}


void Renderer::SetPressedKeys(WPARAM pressedKey, bool flag)
{
    PressedKeys[pressedKey] = flag;
}


void Renderer::OnMouseDown(WPARAM btnState, int x, int y)
{
    if (btnState & MK_LBUTTON)
    {
        m_isMouseRotating = true;
        m_lastMousePos.x = x;
        m_lastMousePos.y = y;
    }
    if (btnState & MK_RBUTTON)
    {
        m_isMouseMovingLight = true;
        m_lastMousePos.x = x;
        m_lastMousePos.y = y;
    }
}


void Renderer::OnMouseUp(WPARAM btnState, int x, int y)
{
    m_isMouseRotating = false;
    m_isMouseMovingLight = false;
}


void Renderer::OnMouseMove(WPARAM btnState, int x, int y)
{
    if (m_isMouseRotating)
    {
        float dx = (float)(x - m_lastMousePos.x) * m_mouseSensitivity;
        float dy = (float)(y - m_lastMousePos.y) * m_mouseSensitivity;

        m_camera.phi += dx * CameraRotationSpeed;
        m_camera.theta -= dy * CameraRotationSpeed;

        m_lastMousePos.x = x;
        m_lastMousePos.y = y;
    }
    if (m_isMouseMovingLight && (btnState & MK_RBUTTON))
    {
        float dx = (float)(x - m_lastMousePos.x) * m_mouseSensitivity;
        float dy = (float)(y - m_lastMousePos.y) * m_mouseSensitivity;

        float dirX = cosf(m_camera.theta) * cosf(m_camera.phi);
        float dirY = sinf(m_camera.theta);
        float dirZ = cosf(m_camera.theta) * sinf(m_camera.phi);

        float upTheta = m_camera.theta + (float)M_PI / 2;
        float upX = cosf(upTheta) * cosf(m_camera.phi);
        float upY = sinf(upTheta);
        float upZ = cosf(upTheta) * sinf(m_camera.phi);

        float rightX = dirY * upZ - dirZ * upY;
        float rightY = dirZ * upX - dirX * upZ;
        float rightZ = dirX * upY - dirY * upX;

        float rightLen = sqrtf(rightX * rightX + rightY * rightY + rightZ * rightZ);
        if (rightLen > 0.0f)
        {
            rightX /= rightLen;
            rightY /= rightLen;
            rightZ /= rightLen;
        }

        float moveSpeed = 1.0f;
        m_pScene->lights[0].pos.x += rightX * dx * moveSpeed;
        m_pScene->lights[0].pos.y += rightY * dx * moveSpeed;
        m_pScene->lights[0].pos.z += rightZ * dx * moveSpeed;
        m_pScene->lights[0].pos.x -= upX * dy * moveSpeed;
        m_pScene->lights[0].pos.y -= upY * dy * moveSpeed;
        m_pScene->lights[0].pos.z -= upZ * dy * moveSpeed;

        m_lastMousePos.x = x;
        m_lastMousePos.y = y;
    }
}
