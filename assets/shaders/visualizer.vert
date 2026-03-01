#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;

uniform mat4 view;
uniform mat4 projection;
uniform float uBass;
uniform float uMid;
uniform float uTreble;

out vec3 Color;

void main() {
    vec3 pos = aPos;
    
    // Determine the distance from the center of the grid
    float dist = length(aOffset.xz);
    float heightMod = 0.0;
    
    // Assign color and height modifier based on radial distance
    if (dist < 10.0) {
        heightMod = uBass * 50.0;
        Color = vec3(1.0, 0.2 + uBass, 0.2); // Red-ish for Bass center
    } else if (dist < 25.0) {
        heightMod = uMid * 20.0;
        Color = vec3(0.2, 1.0, 0.2 + uMid);  // Green-ish for Mids
    } else {
        heightMod = uTreble * 10.0;
        Color = vec3(0.2 + uTreble, 0.2, 1.0); // Blue-ish for Treble edges
    }

    // Only stretch the top vertices of the cube upwards
    if (pos.y > 0.0) {
        pos.y += heightMod;
    }
    
    vec4 worldPos = vec4(pos + aOffset, 1.0);
    gl_Position = projection * view * worldPos;
}
