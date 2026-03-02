#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;

uniform mat4 view;
uniform mat4 projection;
uniform float uSubBass;
uniform float uBass;
uniform float uLowMid;
uniform float uMid;
uniform float uHighMid;
uniform float uPresence;
uniform float uTreble;

uniform float uSubBassMaxDur;
uniform float uBassMaxDur;
uniform float uLowMidMaxDur;
uniform float uMidMaxDur;
uniform float uHighMidMaxDur;
uniform float uPresenceMaxDur;
uniform float uTrebleMaxDur;

out vec3 Color;

void main() {
    vec3 pos = aPos;
    
    // Determine the distance from the center of the grid
    float dist = length(aOffset.xz);
    float heightMod = 0.0;
    
    // How many frames a band must be maxed out before it triggers the effect
    float baseFlashThreshold = 15.0; 
    
    // Spread 7 layers smoothly across the 100x100 grid (max radius ~150)
    if (dist < 15.0) { // Sub-Bass
        heightMod = min(uSubBass * 4.0, 45.0); 
        Color = vec3(0.9, 0.1, 0.1); 
        if (uSubBassMaxDur > baseFlashThreshold * 3.0) Color = vec3(1.0, 1.0, 0.8); // Higher threshold, soft yellow flash
    } else if (dist < 35.0) { // Bass
        heightMod = min(uBass * 3.5, 35.0);
        Color = vec3(0.9, 0.5, 0.1); 
        if (uBassMaxDur > baseFlashThreshold * 2.5) Color = vec3(1.0, 1.0, 0.9); // Higher threshold
    } else if (dist < 55.0) { // Low Mids
        heightMod = min(uLowMid * 3.0, 30.0);
        Color = vec3(0.7, 0.8, 0.1);  
        if (uLowMidMaxDur > baseFlashThreshold) Color = vec3(1.0, 1.0, 1.0);
    } else if (dist < 75.0) { // Mids
        heightMod = min(uMid * 2.5, 25.0);
        Color = vec3(0.2, 0.9, 0.2);  
        if (uMidMaxDur > baseFlashThreshold) Color = vec3(1.0, 1.0, 1.0);
    } else if (dist < 95.0) { // High Mids
        heightMod = min(uHighMid * 2.0, 20.0);
        Color = vec3(0.2, 0.8, 0.9);  
        if (uHighMidMaxDur > baseFlashThreshold) Color = vec3(1.0, 1.0, 1.0);
    } else if (dist < 115.0) { // Presence
        heightMod = min(uPresence * 1.5, 15.0);
        Color = vec3(0.2, 0.3, 0.9); 
        if (uPresenceMaxDur > baseFlashThreshold) Color = vec3(1.0, 1.0, 1.0);
    } else { // Treble
        heightMod = min(uTreble * 1.2, 10.0);
        Color = vec3(0.6, 0.1, 0.8); 
        if (uTrebleMaxDur > baseFlashThreshold) Color = vec3(1.0, 1.0, 1.0);
    }

    // Taper off the height based on distance so the extreme edges are naturally flatter
    float taper = max(0.0, 1.0 - (dist / 140.0));
    heightMod *= taper;

    // Only stretch the top vertices of the cube upwards
    if (pos.y > 0.0) {
        pos.y += heightMod;
    }
    
    vec4 worldPos = vec4(pos + aOffset, 1.0);
    gl_Position = projection * view * worldPos;
}
