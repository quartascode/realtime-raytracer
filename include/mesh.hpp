#include <raymath.h>
#include <vector>

struct triangle {
	Vector2 a;
	Vector2 b;
	Vector2 c;
};

struct mesh {
	std::vector<triangle> triangles;
	int materialIdx;
};
