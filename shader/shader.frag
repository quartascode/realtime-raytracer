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

const int LAMBERTIAN   = 0;
const int METAL        = 1;
const int DIELECTRIC   = 2;
const int LIGHT_SOURCE = 3;

struct material {
	int type;
	vec3 albedo;
    float fuzz;
    float refractionIndex;
    vec3 emissionColor;
    float emissionStrength;
};

struct sphere {
    vec3 center;
    float radius;
	material material;
};
const int SPHERE_COUNT = 4;
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
	material material;
};

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

float RandNormalDist() {
	float theta = 2 * PI * Rand();
	float rho = sqrt(-2 * log(Rand()));
	return rho * cos(theta);
}

vec3 RandVec3() {
	return vec3(Rand(), Rand(), Rand());
}

vec3 MinMaxRandVec3(float min, float max) {
	return vec3(MinMaxRand(min, max), MinMaxRand(min, max), MinMaxRand(min, max));
}

vec3 RandUnitVec() {
	float x = RandNormalDist();
	float y = RandNormalDist();
	float z = RandNormalDist();
	return normalize(vec3(x, y, z));
}

vec3 RandOnHemisphere(vec3 normal) {
	vec3 onUnitSphere = RandUnitVec();
	if (dot(onUnitSphere, normal) < 0) {
		return -onUnitSphere;
	}
	return onUnitSphere;
}

bool Vec3NearZero(vec3 v) {
	float s = 1e-6;
	return (abs(v.x) < s) && (abs(v.y) < s) && (abs(v.z) < s);
}

vec3 Reflect(vec3 v, vec3 n) {
	return v - 2 * dot(v, n) * n;
}

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
	info.material = s.material;

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

void LambertianScatter(inout hitInfo info, inout vec3 attenuation, inout ray scattered) {
	vec3 scatterDirection = info.normal + RandUnitVec();

	if (Vec3NearZero(scatterDirection)) {
		scatterDirection = info.normal;
	}

	scattered = ray(info.hitPoint, scatterDirection);
	attenuation = info.material.albedo;
}

void MetalScatter(ray r, inout hitInfo info, inout vec3 attenuation, inout ray scattered) {
	vec3 reflected = reflect(r.direction, info.normal);
	reflected = normalize(reflected) + (info.material.fuzz * RandUnitVec());
	scattered = ray(info.hitPoint, reflected);
	attenuation = info.material.albedo;
}

vec3 RayColor(in ray r) {
	vec3 color = vec3(1.0);

	for (int i = 0; i <= bounceLimit; i++) {
		hitInfo info;
		if (Hit(r, 0.001, INFINITY, info)) {
			vec3 attenuation;
			vec3 incomingLight = vec3(0);

			switch (info.material.type) {
				case LAMBERTIAN:
					LambertianScatter(info, attenuation, r);
					color *= attenuation;
					break;
				case METAL:
					MetalScatter(r, info, attenuation, r);
					color *= attenuation;
					break;
				case DIELECTRIC:
					break;
				case LIGHT_SOURCE:
					vec3 emmitedLight = info.material.emissionColor * info.material.emissionStrength;
					incomingLight += emmitedLight;
					return incomingLight * color;
					break;
			}
		} else {
			vec3 unitDir = normalize(r.direction);
			float a = 0.5 * (unitDir.y + 1.0);

			vec3 background = (1.0 - a) * vec3(0.7, 0.7, 0.7) + a * vec3(0.3, 0.5, 0.9);
			//vec3 background = vec3(0.0);

			return color * background;
		}
	}
	return vec3(0.0);
}

void main() {
	ivec2 pixel = ivec2(gl_FragCoord.xy);

	material blueLamb;
	blueLamb.type = LAMBERTIAN;
	blueLamb.albedo = vec3(0.1, 0.1, 1);

	material redLamb = blueLamb;
	redLamb.albedo = vec3(1, 0.1, 0.1);

	material greyLamb = blueLamb;
	greyLamb.albedo = vec3(0.651, 0.651, 0.651);

	material whiteLamb = blueLamb;
	whiteLamb.albedo = vec3(1);

	material light;
	light.type = LIGHT_SOURCE;
	light.emissionColor = vec3(1.0, 1.0, 1.0);
	light.emissionStrength = 5.0;

	material mirror;
	mirror.type = METAL;
	mirror.albedo = vec3(1);	
	mirror.fuzz = 0;

	material gold = mirror;
	gold.albedo = vec3(0.827, 0.686, 0.245);
	gold.fuzz = 0.4;

	spheres[0] = sphere(vec3(0, 0, -1.75), 0.5, gold);
	spheres[1] = sphere(vec3(0, -100.5, -1), 100, greyLamb);
	spheres[2] = sphere(vec3(1, 0, -1.5), 0.5, blueLamb);
	spheres[3] = sphere(vec3(-1, 0, -1.5), 0.5, redLamb);

	rngState = uint(pixel.y) * uint(1920) + uint(pixel.x) + uint(frameIndex) * 0x9e3779b9u;

	vec3 pixelColor = vec3(0, 0, 0);
	for (int i = 0; i < samplesPerPixel; i++) {
		ray r = GetRay(pixel.x, pixel.y);
		pixelColor += RayColor(r);
	}

	pixelColor *= pixelSampleScale;

	vec3 previousColor = texelFetch(previousFrame, pixel, 0).rgb;

	float frame = float(frameIndex);

	float weight = 1.0 / (frame + 1);
	vec3 accumulated = previousColor * (1 - weight) + pixelColor * weight;

	//vec3 accumulated = (previousColor * frame + pixelColor) / (frame + 1.0);

	fragColor = vec4(accumulated, 1.0);

}
