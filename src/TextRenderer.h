#include "Game.h"
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
    GLuint Advance;     // Horizontal offset to advance to next glyph
};

class TextRenderer {

public:
    std::map<char, Character> Characters;
    GLuint VAO, VBO;

    TextRenderer(const std::string& fontPath, int fontSize);

    void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);
};