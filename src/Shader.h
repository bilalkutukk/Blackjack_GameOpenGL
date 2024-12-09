#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glad/glad.h>

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    void use() const;
    GLuint getID() const;

private:
    GLuint ID;
    void checkCompileErrors(GLuint shader, const std::string& type);
};

#endif
