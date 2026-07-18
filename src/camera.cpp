#include "camera.hpp"
#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

void SimCamera::Initialize() {
	imageHeight = int(imageWidth / aspectRatio);
	imageHeight = imageHeight < 1 ? 1 : imageHeight;

	float theta = vFov * PI / 180.0f;
	float h = tan(theta / 2);
	float viewportHeight = 2.0 * h * focusDist;
	float viewportWidth = viewportHeight * (float(imageWidth) / imageHeight);

	w = Vector3Normalize(position - lookAt);
	u = Vector3Normalize(Vector3CrossProduct(vup, w));
	v = Vector3CrossProduct(w, u);

	Vector3 viewportU = u * viewportWidth;
	Vector3 viewportV = v * viewportHeight;

	pixelDeltaU = viewportU / imageWidth;
	pixelDeltaV = viewportV / imageHeight;

	Vector3 viewportLowerLeft = position - (w * focusDist) - viewportU/2 - viewportV/2;
	firstPixelPos = viewportLowerLeft + (pixelDeltaU + pixelDeltaV) * 0.5f;

	float alpha = defocusAngle * PI / 180.0f;
	float defocusRadius = focusDist * tan(alpha / 2);
	defocusDiskU = u * defocusRadius;
	defocusDiskV = v * defocusRadius;
}

void SimCamera::Update() {
	pixelDeltaU = Vector3{0, 0, 0};
	pixelDeltaV = Vector3{0, 0, 0};
	firstPixelPos = Vector3{0, 0, 0};

	imageHeight = int(imageWidth / aspectRatio);
	imageHeight = imageHeight < 1 ? 1 : imageHeight;

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

RenderTexture2D SimCamera::LoadFloatRenderTexture(int width, int height) {
	RenderTexture2D target = {};

	target.id = rlLoadFramebuffer();

	if (target.id > 0) {
		rlEnableFramebuffer(target.id);
		target.texture.id = rlLoadTexture(nullptr, width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);

		target.texture.width = width;
		target.texture.height = height;
		target.texture.mipmaps = 1;
		target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;

		rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

		if (!rlFramebufferComplete(target.id)) {
			TraceLog(LOG_ERROR, "Framebuffer float incomplete");
		}

		rlDisableFramebuffer();
	}

	return target;
}
