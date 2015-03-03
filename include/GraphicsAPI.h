
#ifndef GraphicsAPI_H
#define GraphicsAPI_H
#include <opencv2/core/core.hpp>
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")
#include <windows.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "../include/Camera.h"
#include "../include/Model.h"
#include "../include/ColorShader.h"

class ARiftControl;
// -------------------------------

class GraphicsAPI
{
private:
	ID3D11Device* device_;
	ID3D11DeviceContext* devicecontext_;
	DirectX::XMFLOAT4X4 projectionmatrix_;
	DirectX::XMFLOAT4X4 worldmatrix_;
	DirectX::XMFLOAT4X4 orthomatrix_;
	int videocardmemory_;
	char videocarddescription_[128];

	Camera* camera_;
	Model* model_;
	ColorShader* colorshader_;
         
public:
	GraphicsAPI();
	virtual ~GraphicsAPI();
	
	DWORD WINAPI run(LPVOID lpArg);
	bool InitD3D(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
		float screenDepth, float screenNear); 
	bool Frame(ARiftControl* arift_c);
	bool Render(ARiftControl* arift_c);
	void shutDownD3D();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	void GetProjectionMatrix(DirectX::XMFLOAT4X4&);
	void GetWorldMatrix(DirectX::XMFLOAT4X4&);
	void GetOrthoMatrix(DirectX::XMFLOAT4X4&);
	void GetVideoCardInfo(char*, int&);

	// Windows stuff
	HINSTANCE hinstance_;
	HWND window_;
	WNDCLASSEX window_class_;
	LPCWSTR applicationName_;
	unsigned int screenwidth_;
	unsigned int screenheight_;

	// Direct X stuff
	bool vsync_enabled_;
	IDXGISwapChain* swapchain_;
	ID3D11RenderTargetView* rendertargetview_;
	ID3D11Texture2D* depthstencilbuffer_;
	ID3D11DepthStencilState* depthstencilstate_;
	ID3D11DepthStencilView* depthstencilview_;
	ID3D11RasterizerState* rasterstate_;
};

#endif // GraphicsAPI_H
