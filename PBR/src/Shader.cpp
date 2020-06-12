#include "Shader.h"

Shader::Shader(const char* vertex_path, const char* fragment_path, const char* geometry_path) {
	std::string v_str = readShaderFile(vertex_path);
	std::string f_str = readShaderFile(fragment_path);
	std::string g_str = geometry_path ? readShaderFile(geometry_path) : "";
	
	GLint success;
	GLchar log[512];

	const char* v_src = v_str.c_str();
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &v_src, nullptr);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, sizeof(log), nullptr, log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION" << std::endl << log << std::endl;
	}

	const char* f_src = f_str.c_str();
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &f_src, nullptr);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, sizeof(log), nullptr, log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION" << std::endl << log << std::endl;
	}

	GLuint geometry_shader; 
	if (!g_str.empty()) {
		const char* g_src = g_str.c_str();

		geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry_shader, 1, &g_src, nullptr);
		glCompileShader(geometry_shader);
		glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometry_shader, sizeof(log), nullptr, log);
			std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION" << std::endl << log << std::endl;
		}
	}

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	if (!g_str.empty()) {
		glAttachShader(program, geometry_shader);
	}
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, sizeof(log), nullptr, log);
		std::cout << "ERROR::PROGRAM::SHADER::LINKING" << std::endl << log << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	if (!g_str.empty()) {
		glDeleteShader(geometry_shader);
	}
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

void Shader::set_vec2(const char * uniform_name, const GLfloat* value) const {
	glUniform2fv(glGetUniformLocation(program, uniform_name), 1, value);
}

void Shader::set_mat4(const char* uniform_name, const GLfloat* value) const {
	glUniformMatrix4fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, value);
}

void Shader::set_mat3(const char* uniform_name, const GLfloat* value) const {
	glUniformMatrix3fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, value);
}

std::string Shader::readShaderFile(const char* path) const {
	std::ifstream ifStream(path);
	std::string src;

	if (ifStream.is_open()) {
		std::stringstream sstream;

		sstream << ifStream.rdbuf();
		ifStream.close();
		src = sstream.str();
	}
	else {
		std::cout << "ERROR::SHADER::READING_FROM_FILE" << std::endl;
	}

	return src;
}