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

	void Initialize() {

		aspectRatio = 16.0f / 9.0f;
		imageWidth = 1920;

		imageHeight = int(imageWidth / aspectRatio);
		imageHeight = imageHeight < 1 ? 1 : imageHeight;

		vFov = 60;
		float theta = vFov * PI / 180.0f;
		float h = tan(theta / 2);
		float focalLength = 1.0f;
		float viewportHeight = 2.0 * h * focalLength;
		float viewportWidth = viewportHeight * (float(imageWidth) / imageHeight);

		Vector3 viewportU = {viewportWidth, 0, 0};
		Vector3 viewportV = {0, viewportHeight, 0};

		pixelDeltaU = viewportU / imageWidth;
		pixelDeltaV = viewportV / imageHeight;

		Vector3 viewportLowerLeft = position - Vector3{0, 0, focalLength} - viewportU/2 - viewportV/2;
		firstPixelPos = viewportLowerLeft + (pixelDeltaU + pixelDeltaV) * 0.5f;
	}
};
