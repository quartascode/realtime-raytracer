#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;

void main() {
    vec3 color = texture(texture0, fragTexCoord).rgb;

    color = sqrt(color);

    finalColor = vec4(color, 1.0);
}
