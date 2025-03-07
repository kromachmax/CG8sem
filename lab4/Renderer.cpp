
#include "framework.h"
#include "Renderer.h"

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
        result = SetupBackBuffer();
    }

    if (SUCCEEDED(result))
    {
        result = InitScene();
    }

    if (SUCCEEDED(result))
    {
        m_camera.poi = Point{ 0,0,0 };
        m_camera.r = 5.0f;
        m_camera.phi = -(float)M_PI / 4;
        m_camera.theta = (float)M_PI / 4;
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

    if (m_pSceneBuffer)
    {
        m_pSceneBuffer->Release();
        m_pSceneBuffer = nullptr;
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
    m_pDeviceContext->OMSetRenderTargets(1, views, nullptr);

    static const FLOAT BackColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV, BackColor);

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

    m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    ID3D11Buffer* vertexBuffers[] = { m_pVertexBuffer };

    UINT strides[] = { 16 };
    UINT offsets[] = { 0 };

    ID3D11Buffer* cbuffers[] = { m_pSceneBuffer, m_pGeomBuffer };

    m_pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    m_pDeviceContext->VSSetConstantBuffers(0, 2, cbuffers);
    m_pDeviceContext->IASetInputLayout(m_pInputLayout);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
    m_pDeviceContext->DrawIndexed(36, 0, 0);



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

    m_pDeviceContext->UpdateSubresource(m_pGeomBuffer, 0, nullptr, &geomBuffer, 0, 0);

    UpdateCamera(deltaSec);

    m_prevUSec = usec;

    DirectX::XMMATRIX v;
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

        m_pDeviceContext->Unmap(m_pSceneBuffer, 0);
    }

    return SUCCEEDED(result);
}


bool Renderer::Resize(UINT width, UINT height)
{
    if (width != m_width || height != m_height)
    {
        m_pBackBufferRTV->Release();

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

    return result;
}


HRESULT Renderer::InitScene()
{
    HRESULT result;

    static const Vertex Vertices[] = {
    {-0.5f, -0.5f, -0.5f, RGB(255, 0, 0)    },
    { 0.5f, -0.5f, -0.5f, RGB(0, 255, 0)    },
    { 0.5f,  0.5f, -0.5f, RGB(0, 0, 255)    },
    {-0.5f,  0.5f, -0.5f, RGB(255, 255, 0)  },
    {-0.5f, -0.5f,  0.5f, RGB(0, 255, 255)  },
    { 0.5f, -0.5f,  0.5f, RGB(255, 0, 255)  },
    { 0.5f,  0.5f,  0.5f, RGB(255, 255, 255)},
    {-0.5f,  0.5f,  0.5f, RGB(0, 0, 0)      }
    };

    static const USHORT Indices[] = {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };

    static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
       {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
       {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

    desc = {};

    desc.ByteWidth = sizeof(Indices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    data = {};

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
    }


    if (SUCCEEDED(result))
    {
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
    }

    return S_OK;
}


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
        entryPoint = "VS";
        platform = "vs_5_0";
        break;
    }

    case ShaderType::Pixel:
    {
        entryPoint = "PS";
        platform = "ps_5_0";
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


    ID3DBlob* pCode = nullptr;
    ID3DBlob* pErrMsg = nullptr;
    HRESULT result = D3DCompile(data.data(), data.size(), nullptr,
        nullptr, nullptr, entryPoint.c_str(), platform.c_str(),
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

    if (rightLen > 0.0f)
    {
        rightX /= rightLen;
        rightY /= rightLen;
        rightZ /= rightLen;
    }

    float posX = m_camera.poi.x + cosf(m_camera.theta) * cosf(m_camera.phi) * m_camera.r;
    float posY = m_camera.poi.y + sinf(m_camera.theta) * m_camera.r;
    float posZ = m_camera.poi.z + cosf(m_camera.theta) * sinf(m_camera.phi) * m_camera.r;

    bool key = false;

    // Движение вперед (W)
    if (PressedKeys['W'])
    {
        m_camera.r -= cameraSpeed;
        if (m_camera.r < 0.5f)
        {
            m_camera.r = 0.5f;
        }

        PressedKeys['W'] = false;
        return;
    }

    // Движение назад (S)
    if (PressedKeys['S'])
    {
        m_camera.r += cameraSpeed;
        PressedKeys['S'] = false;

        return;
    }



    if (key == true)
    {
        float dx = posX - m_camera.poi.x;
        float dy = posY - m_camera.poi.y;
        float dz = posZ - m_camera.poi.z;

        m_camera.r = sqrtf(dx * dx + dy * dy + dz * dz);

        m_camera.theta = asinf(dy / m_camera.r);
        m_camera.phi = atan2f(dz, dx);
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
}


void Renderer::OnMouseUp(WPARAM btnState, int x, int y)
{
    m_isMouseRotating = false;
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
}
