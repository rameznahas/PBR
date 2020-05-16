#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "glew.h"

class Shader {
public:
	Shader() = default;
	Shader(const char* vertex_path, const char* fragment_path);
	void use();
	void del();
	void set_float(const char* uniform_name, GLfloat value) const;
	void set_int(const char* uniform_name, GLint value) const;
	void set_vec3(const char* uniform_name, const GLfloat* value) const;
	void set_mat4(const char* uniform_name, const GLfloat* value) const;
	void set_mat3(const char* uniform_name, const GLfloat* value) const;
	GLuint id() const { return program; };

private:
	GLuint program;
};
