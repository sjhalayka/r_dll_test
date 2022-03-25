// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"	

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
using namespace std;

#include <cstdlib> // Include this before before glut.h for the sake of MSVC++

#include <GL/glew.h>
#include <GL/glut.h>

// Automatically link in the GLUT and GLEW libraries if compiling on MSVC++
#ifdef _MSC_VER
#pragma comment(lib, "freeglut")
#pragma comment(lib, "glew32")
#endif


bool opengl_init = false;

void init_opengl(void)
{
	if (false == opengl_init)
	{
		int argc = 1; char** argv = 0; argv = new char*; argv[0] = new char[6];
		argv[0][0] = 'w'; argv[0][1] = 'h'; argv[0][2] = 'y'; argv[0][3] = '?'; argv[0][4] = '!'; argv[0][5] = '\0';
		glutInit(&argc, argv);
		delete[] argv[0]; delete argv;

		glutInitDisplayMode(GLUT_RGBA);
		glutCreateWindow("");

		glewInit();

		opengl_init = true;
	}
}


extern "C" __declspec(dllexport) void __cdecl sum(int *len, double *ina, double *inb, double* out)
{
    for(size_t i = 0; i < *len; i++)
        out[i] = ina[i] + inb[i];
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

    return TRUE;
}




bool compile_and_link_compute_shader(const char* const file_name, GLuint& program)
{
	// Read in compute shader contents
	ifstream infile(file_name);

	if (infile.fail())
	{
		cout << "Could not open " << file_name << endl;
		return false;
	}

	string shader_code;
	string line;

	while (getline(infile, line))
	{
		shader_code += line;
		shader_code += "\n";
	}

	// Compile compute shader
	const char* cch = 0;
	GLint status = GL_FALSE;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &(cch = shader_code.c_str()), NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Compute shader compile error.\n";
		vector<GLchar> buf(4096, '\0');
		glGetShaderInfoLog(shader, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDeleteShader(shader);

		return false;
	}

	// Link compute shader
	program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Program link error.\n";
		vector<GLchar> buf(4096, '\0');
		glGetShaderInfoLog(program, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDetachShader(program, shader);
		glDeleteShader(shader);
		glDeleteProgram(program);

		return false;
	}

	// The shader is no longer needed now that the program
	// has been linked
	glDetachShader(program, shader);
	glDeleteShader(shader);

	return true;
}



void calc_sums(size_t index, size_t count, double* ina, double* inb, double* out)
{
	for (size_t i = index; i < (index + count); i++)
		out[i] = ina[i] + inb[i];
}

extern "C" __declspec(dllexport) void __cdecl sum_array(int* len, double* ina, double* inb, double* out)
{
	if (false == opengl_init)
		init_opengl();




	GLuint program = 0;
	GLuint tex_input_a = 0, tex_input_b = 0, tex_output = 0;

	if (false == compile_and_link_compute_shader("sum.cs.glsl", program))
	{
		cout << "Shader sum.cs.glsl compile failure" << endl;
		return;
	}
	else
	{
		glUseProgram(program);
		glGenTextures(1, &tex_output);
		glGenTextures(1, &tex_input_a);
		glGenTextures(1, &tex_input_b);

		cout << "Shader compiled" << endl;
	}


	vector<float> temp_ina;

	for (size_t i = 0; i < *len; i++)
		temp_ina.push_back(ina[i]);

	vector<float> temp_inb;

	for (size_t i = 0; i < *len; i++)
		temp_inb.push_back(inb[i]);

	vector<float> temp_out;

	for (size_t i = 0; i < *len; i++)
		temp_out.push_back(0);




	const size_t num_ops = *len;
	size_t num_ops_remaining = num_ops;

	GLint max_tex_size = 0;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);

	size_t curr_size = max_tex_size;

	while (0 < num_ops_remaining)
	{
		if ((curr_size * curr_size) <= num_ops_remaining)
		{
			const size_t index = num_ops - num_ops_remaining;

			// Generate and allocate output texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_output);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, curr_size, curr_size, 0, GL_RED, GL_FLOAT, NULL);
			glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

			// Generate input textures
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex_input_a);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_input_b);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			// Copy pixel array to GPU as texture 1
			glActiveTexture(GL_TEXTURE1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, curr_size, curr_size, 0, GL_RED, GL_FLOAT, &temp_ina[index]);
			glBindImageTexture(1, tex_input_a, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

			glActiveTexture(GL_TEXTURE2);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, curr_size, curr_size, 0, GL_RED, GL_FLOAT, &temp_inb[index]);
			glBindImageTexture(1, tex_input_b, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);


			// Run compute shader
			glDispatchCompute((GLuint)curr_size, (GLuint)curr_size, 1);

			// Wait for compute shader to finish
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			// Copy output pixel array to CPU as textuare 0
			glActiveTexture(GL_TEXTURE0);
			glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &temp_out[index]);

			num_ops_remaining -= curr_size * curr_size;
		}
		else
		{
			curr_size /= 2;
		}
	}

	
	for (size_t i = 0; i < *len; i++)
		out[i] = static_cast<float>(temp_out[i]);


	glDeleteTextures(1, &tex_output);
	glDeleteTextures(1, &tex_input_a);
	glDeleteTextures(1, &tex_input_b);
	glDeleteProgram(program);
}




