
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "framework.h"
#include "lab1.h"
#include <cassert>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

//--------------------------------------------------------------------------------------
// Глобальные переменные
//--------------------------------------------------------------------------------------

HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;						// Устройство (для создания объектов)
ID3D11DeviceContext*    g_pImmediateContext = NULL;		            // Контекст устройства (рисование)
IDXGISwapChain*         g_pSwapChain = NULL;					    // Цепь связи (буфера с экраном)
ID3D11RenderTargetView* g_pRenderTargetView = NULL;		            // Объект заднего буфера



//--------------------------------------------------------------------------------------
// Предварительные объявления функций
//--------------------------------------------------------------------------------------

HRESULT             InitWindow(HINSTANCE hInstance, int nCmdShow);                  // Создание окна
HRESULT             InitDevice();                                                   // Инициализация устройств DirectX
void                CleanupDevice();                                                // Удаление созданнных устройств DirectX
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);                            // Функция окна
void                Render();                                                       // Функция рисования



//--------------------------------------------------------------------------------------
// Точка входа в программу. Здесь мы все инициализируем и входим в цикл сообщений.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (FAILED(InitWindow(hInstance, nCmdShow)))
        return 0;

    if (FAILED(InitDevice()))
    {
        CleanupDevice();
        return 0;
    }


    // Главный цикл сообщений
    MSG msg = { 0 };

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else                    // Если сообщений нет
        {
            Render();           // Рисуем
        }
    }

    CleanupDevice();

    return (int)msg.wParam;
}

//--------------------------------------------------------------------------------------
// Регистрация класса и создание окна
//--------------------------------------------------------------------------------------

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)

{
    // Регистрация класса
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Lab1";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDC_MYICON);

    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Создание окна
    g_hInst = hInstance;

    RECT rc = { 0, 0, 640, 480 };

    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindow(L"Lab1", L"Lab1", WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Вызывается каждый раз, когда приложение получает сообщение
//--------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:

        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_SIZE:

        if (g_pSwapChain && g_pd3dDevice && g_pImmediateContext)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            if (width == 0 || height == 0)
                return 0;

            if (g_pRenderTargetView)
            {
                g_pRenderTargetView->Release();
                g_pRenderTargetView = nullptr;
            }

            HRESULT hr = g_pSwapChain->ResizeBuffers(
                0,           
                width,      
                height, 
                DXGI_FORMAT_UNKNOWN,
                0
            );

            if (FAILED(hr))
            {
                OutputDebugStringA("Failed to resize swap chain buffers!\n");
                return 0;
            }

            ID3D11Texture2D* pBackBuffer = nullptr;
            hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
            if (SUCCEEDED(hr))
            {
                hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
                pBackBuffer->Release();
                if (FAILED(hr))
                {
                    OutputDebugStringA("Failed to create render target view!\n");
                    return 0;
                }
            }

            D3D11_VIEWPORT viewport = {};
            viewport.Width = (FLOAT)width;
            viewport.Height = (FLOAT)height;
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            g_pImmediateContext->RSSetViewports(1, &viewport);
        }
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    default:

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HRESULT InitDevice()
{
    HRESULT hr = S_OK;
    RECT rc;

    GetClientRect(g_hWnd, &rc);

    UINT width = rc.right - rc.left;              // получаем ширину
    UINT height = rc.bottom - rc.top;             // и высоту окна
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;                // Структура, описывающая цепь связи (Swap Chain)
    ZeroMemory(&sd, sizeof(sd));            // очищаем её

    sd.BufferCount = 2;                                         // у нас один задний буфер
    sd.BufferDesc.Width = width;                                // ширина буфера
    sd.BufferDesc.Height = height;                              // высота буфера
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;          // формат пикселя в буфере
    sd.BufferDesc.RefreshRate.Numerator = 75;                   // частота обновления экрана
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;           // назначение буфера - задний буфер
    sd.OutputWindow = g_hWnd;                                   // привязываем к нашему окну
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;                                         // не полноэкранный режим
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];

#if defined(_DEBUG)
        // Включаем отладочный слой в Debug-сборке
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

        if (SUCCEEDED(hr))                                      // Если устройства созданы успешно, выходим из цикла
            break;
    }

    // Проверка отладочного слоя и вывод всех сообщений
    ID3D11Debug* debugDevice = nullptr;
    hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugDevice);
    if (SUCCEEDED(hr) && debugDevice)
    {
        ID3D11InfoQueue* infoQueue = nullptr;
        hr = debugDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue);
        if (SUCCEEDED(hr) && infoQueue)
        {
            infoQueue->SetMuteDebugOutput(FALSE); // Убеждаемся, что вывод не заглушён

            // Вариант 2: Явно указываем все уровни сообщений
            D3D11_MESSAGE_SEVERITY severities[] = {
                D3D11_MESSAGE_SEVERITY_CORRUPTION, // Критические ошибки
                D3D11_MESSAGE_SEVERITY_ERROR,      // Ошибки
                D3D11_MESSAGE_SEVERITY_WARNING,    // Предупреждения
                D3D11_MESSAGE_SEVERITY_INFO        // Информационные сообщения
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.AllowList.NumSeverities = 4;           // Количество уровней
            filter.AllowList.pSeverityList = severities;  // Список уровней
            infoQueue->PushStorageFilter(&filter);        // Применяем фильтр

            // Выводим все сообщения из очереди
            UINT64 messageCount = infoQueue->GetNumStoredMessages();
            for (UINT64 i = 0; i < messageCount; i++)
            {
                SIZE_T messageLength = 0;
                infoQueue->GetMessage(i, nullptr, &messageLength); // Получаем размер сообщения
                D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(messageLength);
                infoQueue->GetMessage(i, message, &messageLength); // Получаем само сообщение
                if (message != nullptr)
                {
                    OutputDebugStringA(message->pDescription); // Выводим описание в Output
                    OutputDebugStringA("\n");                  // Добавляем перенос строки
                }
                free(message); // Освобождаем память
            }
            infoQueue->Release();
        }
        debugDevice->Release();
    }

    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to get back buffer\n");
        return hr;
    }

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create render target view\n");
        return hr;
    }

    // Подключаем объект заднего буфера к контексту устройства
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

    // Настройка вьюпорта
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

    // Подключаем вьюпорт к контексту устройства
    g_pImmediateContext->RSSetViewports(1, &vp);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Удалить все созданные объекты
//--------------------------------------------------------------------------------------

void CleanupDevice()
{
    // Сначала отключим контекст устройства, потом отпустим объекты.
    if (g_pImmediateContext) g_pImmediateContext->ClearState();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}

//--------------------------------------------------------------------------------------
// Рисование кадра
//--------------------------------------------------------------------------------------

void Render()
{
    g_pImmediateContext->ClearState();

    ID3D11RenderTargetView* views[] = {g_pRenderTargetView};
    g_pImmediateContext->OMSetRenderTargets(1, views, nullptr);


    // Просто очищаем задний буфер
    float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };           // красный, зеленый, синий, альфа-канал
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

    HRESULT result = g_pSwapChain->Present(0, 0);
    assert(SUCCEEDED(result));
}



