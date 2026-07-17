#version 330

out vec4 fragColor;

uniform vec3 firstPixelPos;
uniform vec3 cameraPosition;
uniform vec3 pixelDeltaU;
uniform vec3 pixelDeltaV;
uniform int frameIndex;
uniform sampler2D previousFrame;
uniform int samplesPerPixel;
uniform float pixelSampleScale;
uniform int bounceLimit;

const float PI = 3.14159265358979323846;
const float INFINITY = 1e30;


float DegreeToRadians(float theta) {
	return theta * PI / 180.0;
}

uint rngState;
uint Hash(uint x) {
    x = ((x >> 16) ^ x) * 0x45d9f3bU;
    x = ((x >> 16) ^ x) * 0x45d9f3bU;
    x = (x >> 16) ^ x;
    return x;
}

float Rand() {
	rngState = Hash(rngState);
	if (rngState == 0u) { rngState = 1u; }
	return float(rngState) / (4294967295.0 + 1);
}

float MinMaxRand(float min, float max) {
	return min + (max - min) * Rand();
}

vec3 RandVec3() {
	return vec3(Rand(), Rand(), Rand());
}

vec3 MinMaxRandVec3(float min, float max) {
	return vec3(MinMaxRand(min, max), MinMaxRand(min, max), MinMaxRand(min, max));
}

vec3 RandUnitVec() {
	while (true) {
		vec3 v = MinMaxRandVec3(-1.0, 1.0);
		float lensqrd = dot(v, v);
		if (lensqrd <= 1 && lensqrd > 1e-8) {
			return v / sqrt(lensqrd);
		}
	}
}

vec3 RandOnHemisphere(vec3 normal) {
	vec3 onUnitSphere = RandUnitVec();
	if (dot(onUnitSphere, normal) < 0) {
		return -onUnitSphere;
	}
	return onUnitSphere;
}

struct sphere {
    vec3 center;
    float radius;
};
const int SPHERE_COUNT = 2;
sphere spheres[SPHERE_COUNT];


struct ray {
	vec3 origin;
	vec3 direction;
};

struct hitInfo {
	vec3 hitPoint;
	vec3 normal;
	float t;
	bool frontFace;
};

vec3 RayAt(in ray r, float t) {
	return r.origin + t * r.direction;
}

vec3 SampleSquare() {
	return vec3(Rand() - 0.5, Rand() - 0.5, 0);
}

ray GetRay(int x, int y) {
	vec3 offset = SampleSquare();
	vec3 pixelSample = firstPixelPos + ((x + offset.x) * pixelDeltaU) + ((y + offset.y) * pixelDeltaV);

	vec3 rayOrigin = cameraPosition;
	vec3 rayDirection = pixelSample - rayOrigin;

	return ray(rayOrigin, rayDirection);
}

void SetFaceNormal (ray r, vec3 outwardNormal, inout hitInfo inf) {
	// NOTE: outwardNormal is assumed to be normalized

	inf.frontFace = dot(r.direction, outwardNormal) < 0;
	inf.normal = inf.frontFace ? outwardNormal : -outwardNormal;
}

bool HitSphere(sphere s, ray r, float tMin, float tMax, inout hitInfo info) {
	vec3 oc = s.center - r.origin;

	float a = dot(r.direction, r.direction);
	float h = dot(r.direction, oc);
	float c = dot(oc, oc) - s.radius * s.radius;

	float discriminant = h*h - a*c;

	if (discriminant < 0) {
		return false;
	}

	float root = (h - sqrt(discriminant)) / a;
	if (root <= tMin || root >= tMax) {
		root = (h + sqrt(discriminant)) / a;
		if (root <= tMin || root >= tMax) {
			return false;
		}
	}

	info.t = root;
	info.hitPoint = RayAt(r, info.t);
	vec3 outwardNormal = (info.hitPoint - s.center) / s.radius;
	SetFaceNormal(r, outwardNormal, info);

	return true;
}

bool Hit(ray r, float tMin, float tMax, inout hitInfo info) {
	hitInfo tempInfo;
	bool hitAnything = false;
	float closestSoFar = tMax;

	for (int i = 0; i < SPHERE_COUNT; i++) {
		if (HitSphere(spheres[i], r, tMin, closestSoFar, tempInfo)) {
			hitAnything = true;
			closestSoFar = tempInfo.t;
			info = tempInfo;
		}
	}
	return hitAnything;
}

vec3 RayColor(in ray r) {
	vec3 color = vec3(1.0);

	for (int i = 0; i < bounceLimit; i++) {
		hitInfo info;
		if (Hit(r, 0.001, INFINITY, info)) {
			vec3 direction = info.normal + RandUnitVec();

			color *= 0.5;

			r.origin = info.hitPoint;
			r.direction = direction;
		} else {
			vec3 unitDir = normalize(r.direction);
			float a = 0.5 * (unitDir.y + 1.0);

			vec3 background = (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0);

			return color * background;
		}
	}
	return vec3(0.0);
}

void main() {
	ivec2 pixel = ivec2(gl_FragCoord.xy);

	spheres[0] = sphere(vec3(0, 0, -1), 0.5);
	spheres[1] = sphere(vec3(0, -100.5, -1), 100);

	rngState = uint(pixel.y) * uint(1920) + uint(pixel.x) + uint(frameIndex) * 0x9e3779b9u;

	vec3 pixelColor = vec3(0, 0, 0);
	for (int i = 0; i < samplesPerPixel; i++) {
		ray r = GetRay(pixel.x, pixel.y);
		pixelColor += RayColor(r);
	}

	pixelColor *= pixelSampleScale;

	vec3 previousColor = texelFetch(previousFrame, pixel, 0).rgb;

	float frame = float(frameIndex);

	vec3 accumulated = (previousColor * frame + pixelColor) / (frame + 1.0);

	fragColor = vec4(accumulated, 1.0);

}
