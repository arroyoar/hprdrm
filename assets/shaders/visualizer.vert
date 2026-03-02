#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset; // x, layer_index, z

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
    
    // aOffset.y contains the exact layer index (0, 1, 2, 3...)
    float layer = aOffset.y; 
    float dist = length(aOffset.xz);
    
    float activeLayers = 0.0;
    float baseFlashThreshold = 15.0; 
    vec3 baseColor = vec3(0.0);
    
    // Calculate how many layers high the stack should be at this distance
    if (dist < 15.0) { // Sub-Bass
        activeLayers = min(uSubBass * 3.0, 35.0); 
        baseColor = vec3(0.9, 0.1, 0.1); 
        if (uSubBassMaxDur > baseFlashThreshold * 3.0) baseColor = vec3(1.0, 1.0, 0.8);
    } else if (dist < 35.0) { // Bass
        activeLayers = min(uBass * 2.5, 30.0);
        baseColor = vec3(0.9, 0.5, 0.1); 
        if (uBassMaxDur > baseFlashThreshold * 2.5) baseColor = vec3(1.0, 1.0, 0.9);
    } else if (dist < 55.0) { // Low Mids
        activeLayers = min(uLowMid * 2.0, 25.0);
        baseColor = vec3(0.7, 0.8, 0.1);  
        if (uLowMidMaxDur > baseFlashThreshold) baseColor = vec3(1.0, 1.0, 1.0);
    } else if (dist < 75.0) { // Mids
        activeLayers = min(uMid * 1.8, 20.0);
        baseColor = vec3(0.2, 0.9, 0.2);  
        if (uMidMaxDur > baseFlashThreshold) baseColor = vec3(1.0, 1.0, 1.0);
    } else if (dist < 95.0) { // High Mids
        activeLayers = min(uHighMid * 1.5, 15.0);
        baseColor = vec3(0.2, 0.8, 0.9);  
        if (uHighMidMaxDur > baseFlashThreshold) baseColor = vec3(1.0, 1.0, 1.0);
    } else if (dist < 115.0) { // Presence
        activeLayers = min(uPresence * 1.2, 10.0);
        baseColor = vec3(0.2, 0.3, 0.9); 
        if (uPresenceMaxDur > baseFlashThreshold) baseColor = vec3(1.0, 1.0, 1.0);
    } else { // Treble
        activeLayers = min(uTreble * 1.0, 8.0);
        baseColor = vec3(0.6, 0.1, 0.8); 
        if (uTrebleMaxDur > baseFlashThreshold) baseColor = vec3(1.0, 1.0, 1.0);
    }

    // Taper off the outer edges
    float taper = max(0.0, 1.0 - (dist / 140.0));
    activeLayers *= taper;

    // Minimum 1 layer visible so the grid floor always exists
    activeLayers = max(activeLayers, 0.5);

    // Voxel visibility logic
    if (layer > activeLayers) {
        // If this cube's layer is higher than the active layers, collapse it to 0 (invisible)
        pos *= 0.0;
        Color = vec3(0.0);
    } else {
        // Scale down to create gaps between voxels
        pos *= 0.8; 

        // Gradient based on height: lower blocks are darker, top block is brightest
        float gradient = layer / max(activeLayers, 1.0); 
        Color = baseColor * mix(0.15, 1.0, gradient);

        // Highlight the absolute top block of the stack
        if (layer >= activeLayers - 1.0) {
             Color = mix(Color, vec3(1.0), 0.5); // Add a bright white "cap"
             pos *= 1.15; // Make the top block slightly larger for emphasis
        }
    }

    // Calculate actual world position
    float yBase = dist * -0.1; // The bowl curve
    float actualY = yBase + (layer * 1.0); // Spacing between stacked cubes
    
    vec4 worldPos = vec4(pos + vec3(aOffset.x, actualY, aOffset.z), 1.0);
    gl_Position = projection * view * worldPos;
}
