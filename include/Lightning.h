#ifndef _LIGHTNING_H_
#define _LIGHTNING_H_

#include <DirectXMath.h>

using namespace DirectX;

class Lightning
{
	public:
		Lightning();
		Lightning(const Lightning&);
		~Lightning();

		void SetAmbientColor(float, float, float, float);
		void SetDiffuseColor(float, float, float, float);
		void SetDirection(float, float, float);

		XMFLOAT4 GetAmbientColor();
		XMFLOAT4 GetDiffuseColor();
		XMFLOAT3 GetDirection();

	private:
		XMFLOAT4 ambientColor_;
		XMFLOAT4 diffuseColor_;
		XMFLOAT3 direction_;
};

#endif