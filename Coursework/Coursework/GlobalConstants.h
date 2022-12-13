#pragma once
//! base DX11 includes
#include <d3d11.h>
#include <D3D.h>
#include <D3Dcompiler.h>
#include <dxgi.h>
#include <DirectXMath.h>

//! set of invalid values if null is not available
constexpr float k_InvalidFloat = std::numeric_limits<float>::quiet_NaN();
constexpr float k_NullFloat = 0.f;
constexpr XMFLOAT4 k_InvalidFloat4 = XMFLOAT4(k_InvalidFloat, k_InvalidFloat, k_InvalidFloat, k_NullFloat);
constexpr XMFLOAT3 k_InvalidFloat3 = XMFLOAT3(k_InvalidFloat, k_InvalidFloat, k_InvalidFloat);
const std::string k_InvalidString = "";