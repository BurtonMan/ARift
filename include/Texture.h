#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <d3d11.h>
// #include <DirectXTex\DirectXTex.h>
// #include "../../DirectXTex//DDSTextureLoader/DDSTextureLoader.h"
// #include "../../DirectXTex//DDSTextureLoader/DDSTextureLoader.cpp"
#include <DirectXMath.h>
#include <DDSTextureLoader\DDSTextureLoader.h>

using namespace DirectX;

class Texture
{
	private:
		ID3D11ShaderResourceView* texture_;

	public:
		Texture();
		Texture(const Texture&);
		~Texture();

		bool Initialize(ID3D11Device*, WCHAR*);
		void Shutdown();

		ID3D11ShaderResourceView* GetTexture();
};

#endif