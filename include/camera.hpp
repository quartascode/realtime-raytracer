#pragma once

#include <cmath>
#include <raymath.h>

struct SimCamera {
	float aspectRatio = 16.0f / 9.0f;
	int imageWidth = 1920;
	int imageHeight;
	Vector3 position = {0, 0, 0};
	Vector3 firstPixelPos;
	Vector3 pixelDeltaU;
	Vector3 pixelDeltaV;
	float vFov = 60;

	void Initialize();

};
