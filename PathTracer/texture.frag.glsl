#version 330 core

uniform sampler2D tex;

in vec2 TexCoord;
out vec4 fragColor;

void main() {
	fragColor = texture(tex, TexCoord);
	//fragColor = vec4(1, 0, 0, 1);
}