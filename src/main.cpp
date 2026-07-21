#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <string>
#include "camera.hpp"


int main(void) {
	int samplesPerPixel = 3;
	float pixelSampleScale = 1.0 / samplesPerPixel;

	int bounceLimit = 20;

	int frameIndex = 0;

	SimCamera cam;
	cam.Initialize();

	// raylib
	InitWindow(cam.imageWidth, cam.imageHeight, "raytracer");
	SetTargetFPS(60);
	ToggleFullscreen();
	DisableCursor();

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
	int defocusDiskU_loc = GetShaderLocation(frag, "defocusDiskU");
	int defocusDiskV_loc = GetShaderLocation(frag, "defocusDiskV");
	int defocusAngle_loc = GetShaderLocation(frag, "defocusAngle");

	SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, samplesPerPixel_loc, &samplesPerPixel, SHADER_UNIFORM_INT);
	SetShaderValue(frag, pixelSampleScale_loc, &pixelSampleScale, SHADER_UNIFORM_FLOAT);
	SetShaderValue(frag, bounceLimit_loc, &bounceLimit, SHADER_UNIFORM_INT);
	SetShaderValue(frag, defocusDiskU_loc, &cam.defocusDiskU, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, defocusDiskV_loc, &cam.defocusDiskV, SHADER_UNIFORM_VEC3);
	SetShaderValue(frag, defocusAngle_loc, &cam.defocusAngle, SHADER_UNIFORM_FLOAT);

	RenderTexture2D accum[2] = {
		cam.LoadFloatRenderTexture(cam.imageWidth, cam.imageHeight),
		cam.LoadFloatRenderTexture(cam.imageWidth, cam.imageHeight),
	};

	RenderTexture2D output = LoadRenderTexture(cam.imageWidth, cam.imageHeight);

	int current = 0;

	float velocity = 1;
	float speed;

	int frame = 0;

	while (!WindowShouldClose()) {
		int previous = current;
		current = 1 - current;

		if (IsWindowFocused()) {

			float wheel = GetMouseWheelMove();
			if (wheel != 0) {
				cam.vFov += 1 * -wheel;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}

			float dt = GetFrameTime();

			if (IsKeyDown(KEY_LEFT_SHIFT)) {
				speed = velocity * 10;
			} else {
				speed = velocity;
			}
			if (IsKeyDown(KEY_W)) {
				cam.position -= cam.w * speed * dt;
				cam.lookAt   -= cam.w * speed * dt;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}
			if (IsKeyDown(KEY_S)) {
				cam.position += cam.w * speed * dt;
				cam.lookAt   += cam.w * speed * dt;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}
			if (IsKeyDown(KEY_A)) {
				cam.position -= cam.u * speed * dt;
				cam.lookAt   -= cam.u * speed * dt;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}
			if (IsKeyDown(KEY_D)) {
				cam.position += cam.u * speed * dt;
				cam.lookAt   += cam.u * speed * dt;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}
			if (IsKeyDown(KEY_SPACE)) {
				cam.position.y += speed * dt;
				cam.lookAt.y   += speed * dt;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}
			if (IsKeyDown(KEY_C)) {
				cam.position.y -= speed * dt;
				cam.lookAt.y   -= speed * dt;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}
			if (IsKeyPressed(KEY_R)) {
				cam.position = {0, 0, 0};
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				frameIndex = 0;
			}
			if (IsKeyPressed(KEY_LEFT)) {
				cam.focusDist -= 1;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskU_loc, &cam.defocusDiskU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskV_loc, &cam.defocusDiskV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusAngle_loc, &cam.defocusAngle, SHADER_UNIFORM_FLOAT);
				frameIndex = 0;
			}
			if (IsKeyPressed(KEY_RIGHT)) {
				cam.focusDist += 1;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskU_loc, &cam.defocusDiskU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskV_loc, &cam.defocusDiskV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusAngle_loc, &cam.defocusAngle, SHADER_UNIFORM_FLOAT);
				frameIndex = 0;
			}
			if (IsKeyPressed(KEY_UP)) {
				cam.defocusAngle += 1;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskU_loc, &cam.defocusDiskU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskV_loc, &cam.defocusDiskV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusAngle_loc, &cam.defocusAngle, SHADER_UNIFORM_FLOAT);
				frameIndex = 0;
			}
			if (IsKeyPressed(KEY_DOWN)) {
				cam.defocusAngle -= 1;
				cam.Initialize();
				SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskU_loc, &cam.defocusDiskU, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusDiskV_loc, &cam.defocusDiskV, SHADER_UNIFORM_VEC3);
				SetShaderValue(frag, defocusAngle_loc, &cam.defocusAngle, SHADER_UNIFORM_FLOAT);
				frameIndex = 0;
			}
			if (IsKeyPressed(KEY_P)) {
				BeginTextureMode(output);
				ClearBackground(BLACK);

				BeginShaderMode(display);
				DrawTextureRec(accum[current].texture, Rectangle{0, 0, float(cam.imageWidth), -float(cam.imageHeight)}, Vector2{0, 0}, WHITE);
				EndShaderMode();
				EndTextureMode();

				Image image = LoadImageFromTexture(output.texture);
				ImageFlipVertical(&image);

				ExportImage(image, "render.png");
				UnloadImage(image);
			}

			if (frame > 80) {

				Vector2 mouseDelta = GetMouseDelta();
				if (mouseDelta.x != 0 || mouseDelta.y != 0) {
					cam.UpdateLook();
					cam.Initialize();
					SetShaderValue(frag, firstPixel_loc, &cam.firstPixelPos, SHADER_UNIFORM_VEC3);
					SetShaderValue(frag, pixelDeltaU_loc, &cam.pixelDeltaU, SHADER_UNIFORM_VEC3);
					SetShaderValue(frag, pixelDeltaV_loc, &cam.pixelDeltaV, SHADER_UNIFORM_VEC3);
					SetShaderValue(frag, cameraPos_loc, &cam.position, SHADER_UNIFORM_VEC3);
					frameIndex = 0;
				}
			}
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
			DrawText("Blur Int.:", 10, 70, 30, DARKGREEN);
			DrawText(std::to_string((int)cam.defocusAngle).c_str(), 155, 70, 30, DARKGREEN);
			DrawText("Focus Dst.:", 10, 100, 30, DARKGREEN);
			DrawText(std::to_string((int)cam.focusDist).c_str(), 188, 100, 30, DARKGREEN);


		EndDrawing();

		frameIndex++;
		frame++;
	}

	UnloadShader(frag);
	UnloadShader(display);
	CloseWindow();

	return 0;
}
