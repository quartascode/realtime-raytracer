#version 330

out vec4 fragColor;

uniform vec3 firstPixelPos;
uniform vec3 cameraPosition;
uniform vec3 pixelDeltaU;
uniform vec3 pixelDeltaV;

const float PI = 3.14159265358979323846;
const float INFINITY = 1e30;

float DegreeToRadians(float theta) {
	return theta * PI / 180.0;
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
	hitInfo info;

	if (Hit(r, 0, INFINITY, info)) {
		return 0.5 * vec3(info.normal + vec3(1.0));
	}

	vec3 unitDir = normalize(r.direction);
	float a = 0.5 * (unitDir.y + 1.0);
	return (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0);
}

void main() {
	ivec2 pixel = ivec2(gl_FragCoord.xy);

	vec3 pixelCenter = firstPixelPos + (pixel.x * pixelDeltaU) + (pixel.y * pixelDeltaV);
	vec3 rayDir = pixelCenter - cameraPosition;
	ray r = ray(cameraPosition, rayDir);

	spheres[0] = sphere(vec3(0, 0, -1), 0.5);
	spheres[1] = sphere(vec3(0, -100.5, -1), 100);

	vec3 color = RayColor(r);

	fragColor = vec4(color, 1.0);

}
