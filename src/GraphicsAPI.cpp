#include "../include/GraphicsAPI.h"
#include "../include/ARiftControl.h"
#include <iostream>

using namespace DirectX;

GraphicsAPI::GraphicsAPI()
{
	hinstance_ = GetModuleHandle(NULL);
	applicationName_ = L"Oculus Rift AR";

	swapchain_ = 0;
	device_ = 0;
	devicecontext_ = 0;
	rendertargetview_ = 0;
	depthstencilbuffer_ = 0;
	depthstencilstate_ = 0;
	depthstencilview_ = 0;
	rasterstate_ = 0;

	camera_ = 0;
	model_ = 0;
	colorshader_ = 0;
}

GraphicsAPI::~GraphicsAPI()
{
	shutDownD3D();
}

bool GraphicsAPI::InitD3D(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{

	screenwidth_ = screenWidth;
	screenheight_ = screenHeight;

	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes = 0, i = 0, numerator = 0, denominator = 0, stringLength = 0;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	// Store the vsync setting.
	vsync_enabled_ = vsync;
	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i<numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// We now have the numerator and denominator for the refresh rate.The last thing we will retrieve using the adapter is the name of the video card and the amount of memory on the video card.
		// Get the adapter (video card) description.
		result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	videocardmemory_ = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, videocarddescription_, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;
	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if (vsync_enabled_)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;
	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;
	
	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swapchain_, &device_, NULL, &devicecontext_);
	if (FAILED(result))
	{
		return false;
	}

	// Get the pointer to the back buffer.
	result = swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = device_->CreateRenderTargetView(backBufferPtr, NULL, &rendertargetview_);
	if (FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = device_->CreateTexture2D(&depthBufferDesc, NULL, &depthstencilbuffer_);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = device_->CreateDepthStencilState(&depthStencilDesc, &depthstencilstate_);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	devicecontext_->OMSetDepthStencilState(depthstencilstate_, 1);

	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = device_->CreateDepthStencilView(depthstencilbuffer_, &depthStencilViewDesc, &depthstencilview_);
	if (FAILED(result))
	{
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	devicecontext_->OMSetRenderTargets(1, &rendertargetview_, depthstencilview_);

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = device_->CreateRasterizerState(&rasterDesc, &rasterstate_);
	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	devicecontext_->RSSetState(rasterstate_);

	// Setup the viewport for rendering.
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	devicecontext_->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	fieldOfView = (float)XM_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	// projectionmatrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
	XMMATRIX projectionMatrix_XmMat = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
	XMStoreFloat4x4(&projectionmatrix_, projectionMatrix_XmMat);

	// Initialize the world matrix to the identity matrix.
	// worldmatrix_ = XMMatrixIdentity();
	XMMATRIX worldMatrix_XmMat = XMMatrixIdentity();
	XMStoreFloat4x4(&worldmatrix_, worldMatrix_XmMat);

	// Create an orthographic projection matrix for 2D rendering.
	// orthomatrix_ = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);
	XMMATRIX orthoMatrix_XmMat = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);
	XMStoreFloat4x4(&orthomatrix_, orthoMatrix_XmMat);

	// ------------------------------------------- Tutorial 2 ------------------------------------------

	// Create the camera object.
		camera_ = new Camera();
	if (!camera_)
	{
		return false;
	}

	// Set the initial position of the camera.
	camera_->SetPosition(0.0f, 0.0f, -10.0f);

	
	// Create the model object.
	model_ = new Model();
	if (!model_)
	{
		return false;
	}

	// Initialize the model object.
	result = model_->Initialize(device_);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create the color shader object.
	colorshader_ = new ColorShader();
	if (!colorshader_)
	{
		return false;
	}

	// Initialize the color shader object.
	result = colorshader_->Initialize(device_, hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


bool GraphicsAPI::Frame(ARiftControl* arift_c)
{
	bool result;

	// Render the graphics scene.
	result = Render(arift_c);
	if (!result)
	{
		return false;
	}

	return true;
}


bool GraphicsAPI::Render(ARiftControl* arift_c)
{
	XMFLOAT4X4 viewMatrix, projectionMatrix, worldMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera_->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	camera_->GetViewMatrix(viewMatrix);
	std::cout << "[ GraphicsAPI::Render ] - View Matrix | Row # 1 ] : " << viewMatrix._11 << " " << viewMatrix._12 << " " << viewMatrix._13 << " " << viewMatrix._14 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - View Matrix | Row # 2 ] : " << viewMatrix._21 << " " << viewMatrix._22 << " " << viewMatrix._23 << " " << viewMatrix._24 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - View Matrix | Row # 3 ] : " << viewMatrix._31 << " " << viewMatrix._32 << " " << viewMatrix._33 << " " << viewMatrix._34 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - View Matrix | Row # 4 ] : " << viewMatrix._41 << " " << viewMatrix._42 << " " << viewMatrix._43 << " " << viewMatrix._44 << std::endl << " # ------------------------------------------------------------- " << std::endl;
	
	GetWorldMatrix(worldMatrix);
	std::cout << "[ GraphicsAPI::Render ] - World Matrix | Row # 1 ] : " << worldMatrix._11 << " " << worldMatrix._12 << " " << worldMatrix._13 << " " << worldMatrix._14 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - World Matrix | Row # 2 ] : " << worldMatrix._21 << " " << worldMatrix._22 << " " << worldMatrix._23 << " " << worldMatrix._24 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - World Matrix | Row # 3 ] : " << worldMatrix._31 << " " << worldMatrix._32 << " " << worldMatrix._33 << " " << worldMatrix._34 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - World Matrix | Row # 4 ] : " << worldMatrix._41 << " " << worldMatrix._42 << " " << worldMatrix._43 << " " << worldMatrix._44 << std::endl << " # ------------------------------------------------------------- " << std::endl;
	
	GetProjectionMatrix(projectionMatrix);
	std::cout << "[ GraphicsAPI::Render ] - Projection Matrix | Row # 1 ] : " << projectionMatrix._11 << " " << projectionMatrix._12 << " " << projectionMatrix._13 << " " << projectionMatrix._14 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - Projection Matrix | Row # 2 ] : " << projectionMatrix._21 << " " << projectionMatrix._22 << " " << projectionMatrix._23 << " " << projectionMatrix._24 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - Projection Matrix | Row # 3 ] : " << projectionMatrix._31 << " " << projectionMatrix._32 << " " << projectionMatrix._33 << " " << projectionMatrix._34 << std::endl;
	std::cout << "[ GraphicsAPI::Render ] - Projection Matrix | Row # 4 ] : " << projectionMatrix._41 << " " << projectionMatrix._42 << " " << projectionMatrix._43 << " " << projectionMatrix._44 << std::endl << std::endl;

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	model_->Render(devicecontext_);

	// Render the model using the color shader.
	result = colorshader_->Render(devicecontext_, model_->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		std::cout << "[GraphicsAPI::Render] ColorShader could not render! " << std::endl;
		return false;
	}

	// Present the rendered scene to the screen.
	EndScene();

	std::cout << "[GraphicsAPI::Render] ColorShader successfully rendered! " << std::endl;
	return true;
}

void GraphicsAPI::shutDownD3D()
{
	// ------------------------- Tutorial 2 -----------------------------------------

	// Release the color shader object.
	if (colorshader_)
	{
		colorshader_->Shutdown();
		delete colorshader_;
		colorshader_ = 0;
	}

	// Release the model object.
	if (model_)
	{
		model_->Shutdown();
		delete model_;
		model_ = 0;
	}

	// Release the camera object.
	if (camera_)
	{
		delete camera_;
		camera_ = 0;
	}
	// ------------------------- Tutorial 1 -----------------------------------------

	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (swapchain_)
	{
		swapchain_->SetFullscreenState(false, NULL);
	}

	if (rasterstate_)
	{
		rasterstate_->Release();
		rasterstate_ = 0;
	}

	if (depthstencilview_)
	{
		depthstencilview_->Release();
		depthstencilview_ = 0;
	}

	if (depthstencilstate_)
	{
		depthstencilstate_->Release();
		depthstencilstate_ = 0;
	}

	if (depthstencilbuffer_)
	{
		depthstencilbuffer_->Release();
		depthstencilbuffer_ = 0;
	}

	if (rendertargetview_)
	{
		rendertargetview_->Release();
		rendertargetview_ = 0;
	}

	if (devicecontext_)
	{
		devicecontext_->Release();
		devicecontext_ = 0;
	}

	if (device_)
	{
		device_->Release();
		device_ = 0;
	}

	if (swapchain_)
	{
		swapchain_->Release();
		swapchain_ = 0;
	}

	// Remove the window.
	DestroyWindow(window_);
	window_ = NULL;

	// Remove the application instance.
	UnregisterClass(applicationName_, hinstance_);
	hinstance_ = NULL;

	return;
}

void GraphicsAPI::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	devicecontext_->ClearRenderTargetView(rendertargetview_, color);

	// Clear the depth buffer.
	devicecontext_->ClearDepthStencilView(depthstencilview_, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

void GraphicsAPI::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if (vsync_enabled_)
	{
		// Lock to screen refresh rate.
		swapchain_->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		swapchain_->Present(0, 0);
	}

	return;
}

ID3D11Device* GraphicsAPI::GetDevice()
{
	return device_;
}


ID3D11DeviceContext* GraphicsAPI::GetDeviceContext()
{
	return devicecontext_;
}

void GraphicsAPI::GetProjectionMatrix(XMFLOAT4X4& projectionMatrix)
{
	projectionMatrix = projectionmatrix_;
	return;
}


void GraphicsAPI::GetWorldMatrix(XMFLOAT4X4& worldMatrix)
{
	worldMatrix = worldmatrix_;
	return;
}


void GraphicsAPI::GetOrthoMatrix(XMFLOAT4X4& orthoMatrix)
{
	orthoMatrix = orthomatrix_;
	return;
}

void GraphicsAPI::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, videocarddescription_);
	memory = videocardmemory_;
	return;
}