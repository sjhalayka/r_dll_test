// OpenGL 4.3 introduces compute shaders
#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;

layout(binding = 0, r32f) writeonly uniform image2D output_image;
layout(binding = 1, r32f) readonly uniform image2D input_image_a;
layout(binding = 2, r32f) readonly uniform image2D input_image_b;

void main()
{
	// Get global coordinates
	const ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

	float a = imageLoad(input_image_a, pixel_coords).r;
	float b = imageLoad(input_image_b, pixel_coords).r;

	// Assign only the first element, since we're writing to a 
	// single-channel image
	const vec4 output_pixel = vec4(a + b, 0, 0, 0);

	// Store result in output image
	imageStore(output_image, pixel_coords, output_pixel);
}