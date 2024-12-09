#ifndef GAME_H
#define GAME_H

#include <vector>
#include <map>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include "Card.h"
#include "Shader.h"

class Game {
public:
    Game();
    void run();
    void handleMouseClick(float mouseX, float mouseY);
private:
    void loadAssets();
    GLuint loadTexture(const char* path);
    void initializeCardRendering();
    void initializeDeck();
    void shuffleDeck();
    void dealInitialCards();
    void resetGame();
    void resetDeck();

    void render();
    void renderCards(const std::vector<Card>& hand, float startX, float startY, bool hideSecondCard);
    void renderButton(float x, float y, const std::string& textureKey, const std::string& label);
    void handleInput(GLFWwindow* window);
   
    void update();

    int calculateScore(const std::vector<Card>& hand);
    void dealCard(std::vector<Card>& hand);

    Shader* shader;                       // Shader program for rendering
    Shader* textShader;                       // Shader program for rendering
    std::map<std::string, GLuint> textures; // Card textures
    std::vector<Card> playerHand;          // Player's cards
    std::vector<Card> dealerHand;          // Dealer's cards
    std::vector<Card> deck;                // Deck of cards
    int gameState;                         // 0: Menu, 1: Playing, 2: Game Over
    bool playerTurn;                       // Is it the player's turn?

    GLuint VAO, VBO, EBO;                  // VAO and VBO for card rendering
    std::mt19937 rng;                      // Random number generator for shuffling
    bool deckEmpty;                        // Indicates if the deck is empty

    static const std::string vertexShaderSource;
    static const std::string fragmentShaderSource;
    static const std::string TextvertexShaderSource;
    static const std::string TextfragmentShaderSource;
};

#endif
