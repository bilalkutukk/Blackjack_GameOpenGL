#include "Card.h"

Card::Card(std::string name, int value, GLuint textureID)
    : name(name), value(value), textureID(textureID) {}

std::string Card::getName() const {
    return name;
}

int Card::getValue() const {
    return value;
}

GLuint Card::getTextureID() const {
    return textureID;
}
