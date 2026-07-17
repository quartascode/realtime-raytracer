#version 330

out vec4 fragColor;

uniform vec3 firstPixelPos;
uniform vec3 cameraPosition;
uniform vec3 pixelDeltaU;
uniform vec3 pixelDeltaV;

struct ray {
	vec3 origin;
	vec3 direction;
};

struct hitInfo {
	vec3 hitPoint;
	vec3 normal;
	float t;
};

vec3 RayAt(in ray r, float t) {
	return r.origin + t * r.direction;
}

float HitSphere(vec3 center, float radius, ray r) {
	vec3 oc = center - r.origin;
	float a = dot(r.direction, r.direction);
	float h = dot(r.direction, oc);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = h*h - a*c;

	if (discriminant < 0) {
		return -1.0;
	} else {
		return (h - sqrt(discriminant)) / a;
	}
}

vec3 RayColor(in ray r) {
	float t = HitSphere(vec3(0, 0, -1), 0.5, r);

	if (t > 0.0) {
		vec3 N = normalize(RayAt(r, t) - vec3(0, 0, -1));
		return 0.5 * vec3(N.x + 1, N.y + 1, N.z + 1);
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

	vec3 color = RayColor(r);

	fragColor = vec4(color, 1.0);

}
