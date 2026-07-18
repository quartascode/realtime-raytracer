#include "camera.hpp"
#include <rlgl.h>

void SimCamera::Initialize() {

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
