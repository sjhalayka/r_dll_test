// OpenGL 4.3 introduces compute shaders
#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;

layout(binding = 0, r32f) writeonly uniform image2D output_image;
layout(binding = 1, r32f) readonly uniform image2D input_image_a;

void main()
{
	float running_total = 0;
	const ivec2 size = imageSize(input_image_a);

	for(int i = 0; i < size.x; i++)
	{
		for(int j = 0; j < size.y; j++)
		{
			running_total += imageLoad(input_image_a, ivec2(i, j)).x;
		}
	}

	// Store result in output image
	imageStore(output_image, ivec2(gl_GlobalInvocationID.xy), vec4(running_total, 0, 0, 0));
}