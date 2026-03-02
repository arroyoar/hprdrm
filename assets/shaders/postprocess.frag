#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // Simple Post-Processing Effect: Vignette
    // Calculate distance from the center of the screen (0.5, 0.5)
    float dist = distance(TexCoords, vec2(0.5));
    
    // Smoothly darken the edges
    float vignette = smoothstep(0.8, 0.2, dist);
    col *= vignette;
    
    FragColor = vec4(col, 1.0);
}
