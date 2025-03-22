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

struct VertexRect
{
    float x, y, z;
    COLORREF color;
};

namespace RECTANGLE
{
    struct RectGeomBuffer
    {
        DirectX::XMMATRIX m;
        XMFLOAT4 color;
    };

	class Rectangle
	{
    public:

        Rectangle();

        static HRESULT CreateVertexBuffer(ID3D11Device* m_pDevice);
        static HRESULT CreateIndexBuffer(ID3D11Device* m_pDevice);
        HRESULT CreateGeometryBuffer(ID3D11Device* m_pDevice, const std::string& name, const RectGeomBuffer& buffer);

        void CleanupRectangle();

        ID3D11Buffer* GetGeomBuffer() { return m_pRectangleGeomBuffer; };
        static ID3D11Buffer* GetIndexBuffer() { return m_pRectangleIndexBuffer; };
        static ID3D11Buffer* GetVertexBuffer() { return m_pRectangleVertexBuffer; };
        
        XMFLOAT3 GetCenterCoordinate();


        static const D3D11_INPUT_ELEMENT_DESC InputDesc[];
        static const VertexRect Vertices[];
        static const UINT16 Indices[];

    private:

        ID3D11Buffer* m_pRectangleGeomBuffer;
        static ID3D11Buffer* m_pRectangleVertexBuffer;
        static ID3D11Buffer* m_pRectangleIndexBuffer;

        XMFLOAT3 p_mCenterCoordinate;
	};
}

