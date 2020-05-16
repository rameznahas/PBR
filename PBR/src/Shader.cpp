#include "Shader.h"

Shader::Shader(const char* vertex_path, const char* fragment_path) {
	std::string vertex_src, fragment_src;
	std::ifstream v_ifstream, f_ifstream;
	v_ifstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	f_ifstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		v_ifstream.open(vertex_path);
		f_ifstream.open(fragment_path);

		std::stringstream v_sstream, f_sstream;

		v_sstream << v_ifstream.rdbuf();
		f_sstream << f_ifstream.rdbuf();

		v_ifstream.close();
		f_ifstream.close();

		vertex_src = v_sstream.str();
		fragment_src = f_sstream.str();
	} 
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::READING_FROM_FILE" << std::endl;
	}

	GLint success;
	GLchar log[512];
	
	const char* v_src = vertex_src.c_str();
	const char* f_src = fragment_src.c_str();

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(vertex_shader, 1, &v_src, nullptr);
	glShaderSource(fragment_shader, 1, &f_src, nullptr);

	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, sizeof(log), nullptr, log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION" << std::endl << log << std::endl;
	}

	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, sizeof(log), nullptr, log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION" << std::endl << log << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, sizeof(log), nullptr, log);
		std::cout << "ERROR::PROGRAM::SHADER::LINKING" << std::endl << log << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void Shader::use() {
	glUseProgram(program);
}

void Shader::del() {
	glDeleteProgram(program);
}

void Shader::set_float(const char* uniform_name, GLfloat value) const {
	glUniform1f(glGetUniformLocation(program, uniform_name), value);
}

void Shader::set_int(const char* uniform_name, GLint value) const {
	glUniform1i(glGetUniformLocation(program, uniform_name), value);
}

void Shader::set_vec3(const char * uniform_name, const GLfloat* value) const {
	glUniform3fv(glGetUniformLocation(program, uniform_name), 1, value);
}

void Shader::set_mat4(const char* uniform_name, const GLfloat* value) const {
	glUniformMatrix4fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, value);
}

void Shader::set_mat3(const char* uniform_name, const GLfloat* value) const {
	glUniformMatrix3fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, value);
}