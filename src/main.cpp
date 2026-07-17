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

	// raylib
	InitWindow(imageWidth, imageHeight, "raytracer");
	SetTargetFPS(180);
	ToggleFullscreen();

	Shader frag = LoadShader(nullptr, "shader.frag");

	// shader shit
	int firstPixel_loc = GetShaderLocation(frag, "firstPixelPos");
	int cameraPos_loc = GetShaderLocation(frag, "cameraPosition");
	int pixelDeltaU_loc = GetShaderLocation(frag, "pixelDeltaU");
	int pixelDeltaV_loc = GetShaderLocation(frag, "pixelDeltaV");

	SetShaderValue(frag, firstPixel_loc, &firstPixelPos, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, cameraPos_loc, &cameraCenter, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaU_loc, &pixelDeltaU, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaV_loc, &pixelDeltaV, SHADER_UNIFORM_VEC3);

	while (!WindowShouldClose()) {
		
		BeginDrawing();
			ClearBackground(BLACK);

		BeginShaderMode(frag);
			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
		EndShaderMode();

		//DrawFPS(10, 10);
		EndDrawing();
	}

	UnloadShader(frag);
	CloseWindow();

	return 0;
}
