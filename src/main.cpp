#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

RenderTexture2D LoadFloatRenderTexture(int width, int height) {
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

float DegreeToRadians(float theta) {
	return theta * PI / 180.0;
}



int main(void) {

	float aspectRatio = 16.0f / 9.0f;
	int imageWidth = 1920;

	int imageHeight = int(imageWidth / aspectRatio);
	imageHeight = imageHeight < 1 ? 1 : imageHeight;

	float vFov = 60;
	float theta = DegreeToRadians(vFov);
	float h = tan(theta / 2);

	float focalLength = 1.0f;
	float viewportHeight = 2.0 * h * focalLength;
	float viewportWidth = viewportHeight * (float(imageWidth) / imageHeight);

	Vector3 cameraCenter = {0, 0, 0};

	Vector3 viewportU = {viewportWidth, 0, 0};
	Vector3 viewportV = {0, viewportHeight, 0};

	Vector3 pixelDeltaU = viewportU / imageWidth;
	Vector3 pixelDeltaV = viewportV / imageHeight;

	Vector3 viewportLowerLeft = cameraCenter - Vector3{0, 0, focalLength} - viewportU/2 - viewportV/2;
	Vector3 firstPixelPos = viewportLowerLeft + (pixelDeltaU + pixelDeltaV) * 0.5f;

	int samplesPerPixel = 10;
	float pixelSampleScale = 1.0 / samplesPerPixel;

	int bounceLimit = 50;

	int frameIndex = 0;

	// raylib
	InitWindow(imageWidth, imageHeight, "raytracer");
	SetTargetFPS(0);
	ToggleFullscreen();

	Shader frag = LoadShader(nullptr, "shader/shader.frag");
	Shader display = LoadShader(nullptr, "shader/display.frag");

	// shader shit
	int firstPixel_loc = GetShaderLocation(frag, "firstPixelPos");
	int cameraPos_loc = GetShaderLocation(frag, "cameraPosition");
	int pixelDeltaU_loc = GetShaderLocation(frag, "pixelDeltaU");
	int pixelDeltaV_loc = GetShaderLocation(frag, "pixelDeltaV");
	int frameIndex_loc = GetShaderLocation(frag, "frameIndex");
	int previousFrame_loc = GetShaderLocation(frag, "previousFrame");
	int samplesPerPixel_loc = GetShaderLocation(frag, "samplesPerPixel");
	int pixelSampleScale_loc = GetShaderLocation(frag, "pixelSampleScale");
	int bounceLimit_loc = GetShaderLocation(frag, "bounceLimit");

	SetShaderValue(frag, firstPixel_loc, &firstPixelPos, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, cameraPos_loc, &cameraCenter, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaU_loc, &pixelDeltaU, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaV_loc, &pixelDeltaV, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, samplesPerPixel_loc, &samplesPerPixel, SHADER_UNIFORM_INT);
	SetShaderValue(frag, pixelSampleScale_loc, &pixelSampleScale, SHADER_UNIFORM_FLOAT);
	SetShaderValue(frag, bounceLimit_loc, &bounceLimit, SHADER_UNIFORM_INT);

	RenderTexture2D accum[2] = {
		LoadFloatRenderTexture(imageWidth, imageHeight),
		LoadFloatRenderTexture(imageWidth, imageHeight),
	};

	int current = 0;

	while (!WindowShouldClose()) {
		int previous = current;
		current = 1 - current;

		BeginDrawing();
			ClearBackground(BLACK);
			BeginTextureMode(accum[current]);
			BeginShaderMode(frag);

			SetShaderValueTexture(frag, previousFrame_loc, accum[previous].texture);
			SetShaderValue(frag, frameIndex_loc, &frameIndex, SHADER_UNIFORM_INT);
		
				DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);

			EndShaderMode();
			EndTextureMode();

			BeginShaderMode(display);
				DrawTextureRec(accum[current].texture, Rectangle{0, 0, float(imageWidth), -float(imageHeight)}, Vector2{0, 0}, WHITE);
			EndShaderMode();

			DrawFPS(10, 10);

		EndDrawing();

		frameIndex++;
	}

	UnloadShader(frag);
	UnloadShader(display);
	CloseWindow();

	return 0;
}
