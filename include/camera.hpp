#pragma once

#include <raylib.h>
#include <raymath.h>

struct SimCamera {
	float aspectRatio = 16.0f / 9.0f;
	int imageWidth = 1920;
	int imageHeight;
	Vector3 position = {0, 6, 5};
	Vector3 lookAt = {0, 0, -3};
	Vector3 vup = {0, 1, 0};
	Vector3 u, v, w;
	Vector3 firstPixelPos;
	Vector3 pixelDeltaU;
	Vector3 pixelDeltaV;
	float vFov = 60;

	float sensitivity = 0.1f;
	float yaw = -90.0f;
	float pitch = 0.0f;

	float defocusAngle = 0;
	float focusDist = 1;
	Vector3 defocusDiskU;
	Vector3 defocusDiskV;

	void Initialize();

	void UpdateLook();

	void Update();

	RenderTexture2D LoadFloatRenderTexture(int width, int height);
};
