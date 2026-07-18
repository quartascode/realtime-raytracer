#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <string>
#include "camera.hpp"


int main(void) {
	int samplesPerPixel = 10;
	float pixelSampleScale = 1.0 / samplesPerPixel;

	int bounceLimit = 50;

	int frameIndex = 0;

	SimCamera cam;
	cam.Initialize();

	// raylib
	InitWindow(cam.imageWidth, cam.imageHeight, "raytracer");
	SetTargetFPS(60);
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

	SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, samplesPerPixel_loc, &samplesPerPixel, SHADER_UNIFORM_INT);
	SetShaderValue(frag, pixelSampleScale_loc, &pixelSampleScale, SHADER_UNIFORM_FLOAT);
	SetShaderValue(frag, bounceLimit_loc, &bounceLimit, SHADER_UNIFORM_INT);

	RenderTexture2D accum[2] = {
		cam.LoadFloatRenderTexture(cam.imageWidth, cam.imageHeight),
		cam.LoadFloatRenderTexture(cam.imageWidth, cam.imageHeight),
	};

	int current = 0;

	while (!WindowShouldClose()) {
		int previous = current;
		current = 1 - current;

		float wheel = GetMouseWheelMove();
		if (wheel != 0) {
			cam.vFov += 1 * -wheel;
			cam.Initialize();
			SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
			SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
			SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
			frameIndex = 0;
		}


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
				DrawTextureRec(accum[current].texture, Rectangle{0, 0, float(cam.imageWidth), -float(cam.imageHeight)}, Vector2{0, 0}, WHITE);
			EndShaderMode();

			DrawFPS(10, 10);
			DrawText(std::to_string((int)cam.vFov).c_str(), 80, 40, 30, DARKGREEN);
			DrawText("Fov:", 10, 40, 30, DARKGREEN);


		EndDrawing();

		frameIndex++;
	}

	UnloadShader(frag);
	UnloadShader(display);
	CloseWindow();

	return 0;
}
