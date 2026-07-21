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
uniform vec3 defocusDiskU;
uniform vec3 defocusDiskV;
uniform float defocusAngle;

const float PI = 3.14159265358979323846;
const float INFINITY = 1e30;

const int LAMBERTIAN   = 0;
const int METAL        = 1;
const int DIELECTRIC   = 2;
const int LIGHT_SOURCE = 3;

const bool night = true;

struct material {
	int type;
	vec3 albedo;
	float fuzz;
	float refractionIndex;
	vec3 emissionColor;
	float emissionStrength;
};

struct triangle {
	vec3 a;
	vec3 b;
	vec3 c;
	material material;
	bool backFaceCull;
};
const int TRIANGLE_COUNT = 30;
triangle triangs[TRIANGLE_COUNT];

struct sphere {
	vec3 center;
	float radius;
	material material;
};
const int SPHERE_COUNT = 11;
sphere spheres[SPHERE_COUNT];

struct ray {
	vec3 origin;
	vec3 direction;
};

struct hitInfo {
	bool didHit;
	vec3 hitPoint;
	vec3 normal;
	float t;
	bool frontFace;
	material material;
};

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

vec3 RandInUnitDisk() {
	vec2 dir;
	dir.x = RandNormalDist();
	dir.y = RandNormalDist();

	float radius = sqrt(Rand());

	return vec3(dir * radius, 0);
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

float Reflactance(float cos, float ri) {
	float r0 = (1 - ri) / (1 + ri);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cos), 5);
}

vec3 Refract(vec3 v, vec3 n, float nRatio) {
	float cosTheta = min(dot(-v, n), 1.0);
	vec3 perpendicular = nRatio * (v + cosTheta * n);
	vec3 parallel = -sqrt(abs(1.0 - dot(perpendicular, perpendicular))) * n;
	return perpendicular + parallel;
}

vec3 RayAt(in ray r, float t) {
	return r.origin + t * r.direction;
}

vec3 SampleSquare() {
	return vec3(Rand() - 0.5, Rand() - 0.5, 0);
}

vec3 DefocusDiskSample() {
	vec3 p = RandInUnitDisk();
	return cameraPosition + (p.x * defocusDiskU) + (p.y * defocusDiskV);
}

ray GetRay(int x, int y) {
	vec3 offset = SampleSquare();
	vec3 pixelSample = firstPixelPos + ((x + offset.x) * pixelDeltaU) + ((y + offset.y) * pixelDeltaV);

	vec3 rayOrigin = (defocusAngle <= 0) ? cameraPosition : DefocusDiskSample();
	vec3 rayDirection = pixelSample - rayOrigin;

	return ray(rayOrigin, rayDirection);
}

void SetFaceNormal (ray r, vec3 outwardNormal, inout hitInfo inf) {
	// NOTE: outwardNormal is assumed to be normalized

	inf.frontFace = dot(r.direction, outwardNormal) < 0;
	inf.normal = inf.frontFace ? outwardNormal : -outwardNormal;
}

hitInfo HitSphere(sphere s, ray r, float tMin, float tMax) {
	hitInfo info;

	vec3 oc = s.center - r.origin;

	float a = dot(r.direction, r.direction);
	float h = dot(r.direction, oc);
	float c = dot(oc, oc) - s.radius * s.radius;

	float discriminant = h*h - a*c;

	if (discriminant < 0) {
		info.didHit = false;
		return info;
	}

	float root = (h - sqrt(discriminant)) / a;
	if (root <= tMin || root >= tMax) {
		root = (h + sqrt(discriminant)) / a;
		if (root <= tMin || root >= tMax) {
			info.didHit = false;
			return info;
		}
	}

	info.didHit = true;
	info.t = root;
	info.hitPoint = RayAt(r, info.t);
	vec3 outwardNormal = (info.hitPoint - s.center) / s.radius;
	SetFaceNormal(r, outwardNormal, info);
	info.material = s.material;

	return info;
}

hitInfo HitTriangle(triangle tr, ray r, float tMin, float tMax) {
	float EPSILON = 1e-6;
	float t;

	hitInfo info;
	info.didHit = false;

	vec3 edgeAB = tr.b - tr.a;
	vec3 edgeAC = tr.c - tr.a;

	vec3 pVec = cross(r.direction, edgeAC);

	float det = dot(edgeAB, pVec);

	if (tr.backFaceCull) {
		if (det < EPSILON) {
			return info;
		}

		vec3 tVec = r.origin - tr.a;

		float u = dot(tVec, pVec);
		if (u < 0.0 || u > det) {
			return info;
		}

		vec3 qVec = cross(tVec, edgeAB);

		float v = dot(r.direction, qVec);
		if (v < 0.0 || u + v > det) {
			return info;
		}

		t = dot(edgeAC, qVec);
		float invDet = 1.0 / det;
		t *= invDet;
	} else {
		if (det > -EPSILON && det < EPSILON) {
			return info;
		}
		float invDet = 1.0 / det;

		vec3 tVec = r.origin - tr.a;

		float u = dot(tVec, pVec) * invDet;
		if (u < 0.0 || u > 1.0) {
			return info;
		}

		vec3 qVec = cross(tVec, edgeAB);

		float v = dot(r.direction, qVec) * invDet;
		if (v < 0.0 || u + v > 1.0) {
			return info;
		}

		t = dot(edgeAC, qVec) * invDet;
	}

	if (t < tMin || t > tMax) {
		return info;
	}

	vec3 outwardNormal = normalize(cross(edgeAB, edgeAC));

	info.didHit = true;
	info.hitPoint = RayAt(r, t);
	info.t = t;
	SetFaceNormal(r, outwardNormal, info);
	info.material = tr.material;

	return info;
}

hitInfo Hit(ray r, float tMin, float tMax) {
	hitInfo info;
	hitInfo temp;
	info.didHit = false;
	info.t = tMax;

	for (int i = 0; i < SPHERE_COUNT; i++) {
		temp = HitSphere(spheres[i], r, tMin, info.t);
		if (temp.didHit) {
			info = temp;
		}
	}

	for (int i = 0; i < TRIANGLE_COUNT; i++) {
		temp = HitTriangle(triangs[i], r, tMin, info.t);
		if (temp.didHit) {
			info = temp;
		}
	}

	return info;
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

void DielectricScatter(ray r, inout hitInfo info, inout vec3 attenuation, inout ray scattered) {
	float ri = info.frontFace ? (1.0 / info.material.refractionIndex) : info.material.refractionIndex;

	vec3 unitDir = normalize(r.direction);
	float cosTheta = min(dot(-unitDir, info.normal), 1.0);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	bool cannotRefract = ri * sinTheta > 1.0;
	vec3 dir;

	if (cannotRefract || Reflactance(cosTheta, ri) > Rand()) {
		dir = Reflect(unitDir, info.normal);
	} else {
		dir = Refract(unitDir, info.normal, ri);
	}

	scattered = ray(info.hitPoint, dir);

	attenuation = info.material.albedo;
}

vec3 RayColor(in ray r) {
	vec3 color = vec3(1.0);
	for (int i = 0; i <= bounceLimit; i++) {
		hitInfo info = Hit(r, 0.001, INFINITY);
		if (info.didHit) {
			vec3 attenuation;

			switch (info.material.type) {
				case LAMBERTIAN:
					LambertianScatter(info, attenuation, r);
					break;
				case METAL:
					MetalScatter(r, info, attenuation, r);
					break;
				case DIELECTRIC:
					DielectricScatter(r, info, attenuation, r);
					break;
				case LIGHT_SOURCE:
					vec3 emmitedLight = info.material.emissionColor * info.material.emissionStrength;
					return emmitedLight * color;
			}
			color *= attenuation;
		} else {
			vec3 unitDir = normalize(r.direction);
			float a = 0.5 * (unitDir.y + 1.0);
			
			vec3 background;

			if (night) {
				background = vec3(0.0);
			} else {
				background = (1.0 - a) * vec3(0.7, 0.7, 0.7) + a * vec3(0.3, 0.5, 0.9);
			}

			return color * background;
		}
	}
	return vec3(0.0);
}

void main() {
	ivec2 pixel = ivec2(gl_FragCoord.xy);

	material blue;
	blue.type = LAMBERTIAN;
	blue.albedo = vec3(0.0, 0.0, 1.0);

	material red = blue;
	red.albedo = vec3(1.0, 0.0, 0.0);

	material green = blue;
	green.albedo = vec3(0.0, 1, 0.0);

	material grey = blue;
	grey.albedo = vec3(0.151, 0.151, 0.151);

	material white = blue;
	white.albedo = vec3(1);

	material purple = blue;
	purple.albedo = vec3(0.8, 0.2, 0.8);

	material yellow = blue;
	yellow.albedo = vec3(0.8, 0.8, 0.1);

	material light;
	light.type = LIGHT_SOURCE;
	light.emissionColor = vec3(1.0, 1.0, 1.0);
	light.emissionStrength = 32.0;

	material gl = light;
	gl.emissionColor = green.albedo;

	material bl = light;
	bl.emissionColor = blue.albedo;

	material pl = light;
	pl.emissionColor = purple.albedo;

	material rl = light;
	rl.emissionColor = red.albedo;

	material mirror;
	mirror.type = METAL;
	mirror.albedo = vec3(1);	
	mirror.fuzz = 0;

	material gold = mirror;
	gold.albedo = vec3(0.827, 0.686, 0.245);
	gold.fuzz = 0.1;

	material otherMirror = mirror;
	otherMirror.albedo = vec3(0.3, 0.3, 0.3);

	material glass;
	glass.type = DIELECTRIC;
	glass.refractionIndex = 1.5;
	glass.albedo = vec3(1, 1, 1);

	material inGlass = glass;
	inGlass.refractionIndex = 1/1.5;

	// parede vermelha
	triangs[0] = triangle(vec3(1.2, 0, -3), vec3(-1.2, 0, -3), vec3(1.2, 4, -3), red, false);
	triangs[1] = triangle(vec3(1.2, 4, -3), vec3(-1.2, 0, -3), vec3(-1.2, 4, -3), red, false);

	// parede cinza de tras
	triangs[2] = triangle(vec3(-1.2, 4, -3), vec3(-1.2, 0, -3), vec3(-1.2, 4, -5.4), grey, false);
	triangs[3] = triangle(vec3(-1.2, 4, -5.4), vec3(-1.2, 0, -3), vec3(-1.2, 0, -5.4), grey, false);

	// parede azul
	triangs[4] = triangle(vec3(1.2, 0, -5.4), vec3(1.2, 4, -5.4), vec3(-1.2, 4, -5.4), blue, false);
	triangs[5] = triangle(vec3(-1.2, 0, -5.4), vec3(1.2, 0, -5.4), vec3(-1.2, 4, -5.4), blue, false);

	// teto branco
	triangs[6] = triangle(vec3(-1.2, 4, -3.0), vec3(-1.2, 4, -5.4), vec3(1.2, 4, -3), white, false);
	triangs[7] = triangle(vec3(1.2, 4, -5.4), vec3(1.2, 4, -3), vec3(-1.2, 4, -5.4), white, false);

	// chao verde
	triangs[8] = triangle(vec3(-1.2, 0, -3.0), vec3(1.2, 0, -3), vec3(-1.2, 0, -5.4), green, false);
	triangs[9] = triangle(vec3(1.2, 0, -5.4), vec3(-1.2, 0, -5.4), vec3(1.2, 0, -3), green, false);

	// luz
	triangs[10] = triangle(vec3(0.3, 3.995, -3 - 0.9), vec3(-0.3, 3.995, -3.0 - 0.9), vec3(-0.3, 3.995, -5.4 + 0.9), light, false);
	triangs[11] = triangle(vec3(0.3, 3.995, -3 - 0.9), vec3(0.3, 3.995, -5.4 + 0.9), vec3(-0.3, 3.995, -5.4 + 0.9), light, false);

	// parede branca invisivel pra camera
	triangs[12] = triangle(vec3(1.2, 0, -3), vec3(1.2, 4, -3), vec3(1.2, 4, -5.4), grey, true);
	triangs[13] = triangle(vec3(1.2, 0, -3), vec3(1.2, 4, -5.4), vec3(1.2, 0, -5.4), grey, true);

	// ----------- //

//	spheres[0] = sphere(vec3(-0.65, 0.8, -3.95), 0.5, glass);
//	spheres[1] = sphere(vec3(-0.1, 3.43, -4.61), 0.5, glass);
//	spheres[2] = sphere(vec3(0.63, 2.43, -3.6), 0.5, glass);
//	spheres[3] = sphere(vec3(0, 1.73, -4.8), 0.5, glass);
//
//	spheres[4] = sphere(vec3(-0.75, 3.5, -3.5), 0.2, glass);
//	spheres[5] = sphere(vec3(-0.1, 2.63, -4.61), 0.2, glass);
//	spheres[6] = sphere(vec3(1.13, 1.63, -3.3), 0.2, glass);
//	spheres[7] = sphere(vec3(0.3, 0.83, -5.0), 0.2, glass);

	// +X
	triangs[14] = triangle(vec3(0.4, 0.0, -3.9), vec3(0.4, 0.0, -4.8), vec3(0.4, 0.9, -4.8), white, false);
	triangs[15] = triangle(vec3(0.4, 0.0, -3.9), vec3(0.4, 0.9, -4.8), vec3(0.4, 0.9, -3.9), white, false);

	// -X
	triangs[16] = triangle(vec3(-0.5, 0.0, -4.8), vec3(-0.5, 0.0, -3.9), vec3(-0.5, 0.9, -3.9), white, false);
	triangs[17] = triangle(vec3(-0.5, 0.0, -4.8), vec3(-0.5, 0.9, -3.9), vec3(-0.5, 0.9, -4.8), white, false);

	// +Y
	triangs[18] = triangle(vec3(-0.5, 0.9, -3.9), vec3(0.4, 0.9, -3.9), vec3(0.4, 0.9, -4.8), white, false);
	triangs[19] = triangle(vec3(-0.5, 0.9, -3.9), vec3(0.4, 0.9, -4.8), vec3(-0.5, 0.9, -4.8), white, false);

	// -Y
	triangs[20] = triangle(vec3(-0.5, 0.0, -4.8), vec3(0.4, 0.0, -4.8), vec3(0.4, 0.0, -3.9), white, false);
	triangs[21] = triangle(vec3(-0.5, 0.0, -4.8), vec3(0.4, 0.0, -3.9), vec3(-0.5, 0.0, -3.9), white, false);

	// -Z
	triangs[22] = triangle(vec3(0.4, 0.0, -4.8), vec3(-0.5, 0.0, -4.8), vec3(-0.5, 0.9, -4.8), white, false);
	triangs[23] = triangle(vec3(0.4, 0.0, -4.8), vec3(-0.5, 0.9, -4.8), vec3(0.4, 0.9, -4.8), white, false);

	// +Z
	triangs[24] = triangle(vec3(-0.5, 0.0, -3.9), vec3(0.4, 0.0, -3.9), vec3(0.4, 0.9, -3.9), white, false);
	triangs[25] = triangle(vec3(-0.5, 0.0, -3.9), vec3(0.4, 0.9, -3.9), vec3(-0.5, 0.9, -3.9), white, false);

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
