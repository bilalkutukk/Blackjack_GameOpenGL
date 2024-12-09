#ifndef CARD_H
#define CARD_H

#include <string>
#include <glad/glad.h>

class Card {
public:
    Card(std::string name, int value, GLuint textureID);
    std::string getName() const;
    int getValue() const;
    GLuint getTextureID() const;

private:
    std::string name;
    int value;
    GLuint textureID;
};

#endif
