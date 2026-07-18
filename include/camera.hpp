#pragma once

#include <raylib.h>
#include <raymath.h>

struct SimCamera {
	float aspectRatio = 16.0f / 9.0f;
	int imageWidth = 1920;
	int imageHeight;
	Vector3 position = {0, 0, 0};
	Vector3 lookAt = {0, 0, -1};
	Vector3 vup = {0, 1, 0};
	Vector3 u, v, w;
	Vector3 firstPixelPos;
	Vector3 pixelDeltaU;
	Vector3 pixelDeltaV;
	float vFov = 60;

	float defocusAngle = 0;
	float focusDist = 1;
	Vector3 defocusDiskU;
	Vector3 defocusDiskV;

	void Initialize();

	void Update();

	RenderTexture2D LoadFloatRenderTexture(int width, int height);
};
