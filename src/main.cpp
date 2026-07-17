#include <raylib.h>
#include <raymath.h>

int main(void) {

	float aspectRatio = 16.0f / 9.0f;
	int imageWidth = 1920;

	int imageHeight = int(imageWidth / aspectRatio);
	imageHeight = imageHeight < 1 ? 1 : imageHeight;

	float focalLength = 1.0f;
	float viewportHeight = 2.0;
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

	int bounceLimit = 30;

	int frameIndex = 0;

	// raylib
	InitWindow(imageWidth, imageHeight, "raytracer");
	SetTargetFPS(60);
	ToggleFullscreen();

	Shader frag = LoadShader(nullptr, "shader/shader.frag");

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
		LoadRenderTexture(imageWidth, imageHeight),
		LoadRenderTexture(imageWidth, imageHeight),
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

			DrawTextureRec(accum[current].texture, Rectangle{0, 0, float(imageWidth), -float(imageHeight)}, Vector2{0, 0}, WHITE);
			DrawFPS(10, 10);

		EndDrawing();

		frameIndex++;
	}

	UnloadShader(frag);
	CloseWindow();

	return 0;
}
