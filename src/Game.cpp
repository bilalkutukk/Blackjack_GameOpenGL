#include "Game.h"
#include <iostream>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TextRenderer.h"

const std::string Game::TextvertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // (position, texcoords)
out vec2 TexCoords;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

const std::string Game::TextfragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {    
    float alpha = texture(text, TexCoords).r;
    color = vec4(textColor, alpha);
}
)";


// Initialize static members
const std::string Game::vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    uniform mat4 model;

    void main() {
        gl_Position = model * vec4(vec3(aPos.x, -aPos.y, aPos.z), 1.0);
        TexCoord = aTexCoord;
    }
)";

const std::string Game::fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;
    uniform sampler2D texture1;

    void main() {
        FragColor = texture(texture1, TexCoord);
    }
)";

struct Button {
    float x;      // Center x-coordinate
    float y;      // Center y-coordinate
    float width;  // Button width
    float height; // Button height
    std::string action; // Associated action ("hit", "stand", "restart", "resetDeck")
};

std::vector<Button> buttons = {
    { -0.75f, -0.8f, 0.4f, 0.2f, "hit" },       // Hit button
    { -0.25f, -0.8f, 0.4f, 0.2f, "stand" },     // Stand button
    {  0.25f, -0.8f, 0.4f, 0.2f, "restart" },   // Restart button
    {  0.75f, -0.8f, 0.4f, 0.2f, "resetDeck" }  // Reset Deck button
};

TextRenderer* textRenderer;
std::string gameMessage;

Game::Game() : gameState(1), playerTurn(true), rng(std::random_device{}()), deckEmpty(false) {}

GLuint Game::loadTexture(const char* path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

void Game::initializeCardRendering() {
    float vertices[] = {
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f
    };

    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Game::resetDeck() {
    deck.clear();
    initializeDeck();
    shuffleDeck();
    deckEmpty = false;
    resetGame();
}

void Game::loadAssets() {
    textures["cardBack"] = loadTexture("assets/cardBack_blue1.png");
    textures["cardSpadesA"] = loadTexture("assets/cardSpadesA.png");
    textRenderer = new TextRenderer("assets/font.ttf", 24);

    // Enable blending for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Game::initializeDeck() {
    std::vector<std::string> suits = { "Spades", "Hearts", "Clubs", "Diamonds" };
    std::vector<std::string> ranks = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A" };
    std::vector<int> values = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11 };

    for (const auto& suit : suits) {
        for (size_t i = 0; i < ranks.size(); ++i) {
            std::string cardName = ranks[i] + " of " + suit;
            GLuint texture = loadTexture(("assets/card" + suit + ranks[i] + ".png").c_str());
            deck.emplace_back(cardName, values[i], texture);
        }
    }
}

void Game::shuffleDeck() {
    std::shuffle(deck.begin(), deck.end(), rng);
}

void Game::dealInitialCards() {
    if (deck.size() < 4) {
        std::cout << "Not enough cards for a new round. Resetting deck..." << std::endl;
        resetDeck();
        return;
    }
    playerHand.clear();
    dealerHand.clear();
    dealCard(playerHand);
    dealCard(dealerHand);
    dealCard(playerHand);
    dealCard(dealerHand);
}


void Game::resetGame() {
    if (deck.size() < 4) {
        std::cout << "Not enough cards to start a new game. Resetting deck..." << std::endl;
        resetDeck();
        return;
    }
    gameState = 1;
    gameMessage.clear();
    playerTurn = true;
    playerHand.clear();
    dealerHand.clear();
    dealInitialCards();
    std::cout << "Game reset. New round starting!" << std::endl;
    std::cout << "Cards left in deck: " << deck.size() << std::endl;
}



void Game::dealCard(std::vector<Card>& hand) {
    if (deck.empty()) {
        std::cout << "Deck is finished. Restarting the game automatically..." << std::endl;
        resetDeck(); // Automatically reset the deck and restart the game
        return;
    }
    hand.push_back(deck.back());
    deck.pop_back();
    std::cout << "Cards left in deck: " << deck.size() << std::endl;

    if (deck.size() < 4) {
        std::cout << "Warning: Deck is running low. Not enough cards for the next round!" << std::endl;
    }
}



int Game::calculateScore(const std::vector<Card>& hand) {
    int score = 0;
    int aceCount = 0;

    for (const auto& card : hand) {
        score += card.getValue();
        if (card.getValue() == 11) ++aceCount;
    }

    while (score > 21 && aceCount > 0) {
        score -= 10;
        --aceCount;
    }

    return score;
}

void Game::handleInput(GLFWwindow* window) {
    static bool hitPressed = false;
    static bool standPressed = false;
    static bool restartPressed = false;

    if (gameState == 2) {
        // Allow "Hit" button to restart the game when the round ends
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            if (!hitPressed) {
                hitPressed = true;
                resetGame();
                std::cout << "New round started via Hit button!" << std::endl;
            }
        }
        else {
            hitPressed = false;
        }

        // Disable "R" button functionality during the game
        return;
    }

    if (gameState == 1) {
        // Handle "Hit" input during gameplay
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            if (!hitPressed) {
                hitPressed = true;
                dealCard(playerHand);
                int playerScore = calculateScore(playerHand);
                if (playerScore == 21) {
                    std::cout << "Player hits 21! You win!" << std::endl;
                    gameState = 2; // End the game
                }
                else if (playerScore > 21) {
                    std::cout << "Player busts!" << std::endl;
                    gameState = 2; // End the game
                }
            }
        }
        else {
            hitPressed = false;
        }

        // Handle "Stand" input
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (!standPressed) {
                standPressed = true;
                playerTurn = false; // End the player's turn
            }
        }
        else {
            standPressed = false;
        }
    }

    // Handle "Restart" input (only when game is not in progress)
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && gameState != 1) {
        if (!restartPressed) {
            restartPressed = true;
            resetGame();
            std::cout << "Game restarted via Restart button!" << std::endl;
        }
    }
    else {
        restartPressed = false;
    }
}


void Game::handleMouseClick(float mouseX, float mouseY) {
    for (const auto& button : buttons) {
        // Convert mouse coordinates to normalized device coordinates
        float buttonLeft = button.x - button.width / 2;
        float buttonRight = button.x + button.width / 2;
        float buttonTop = button.y + button.height / 2;
        float buttonBottom = button.y - button.height / 2;

        if (mouseX >= buttonLeft && mouseX <= buttonRight &&
            mouseY >= buttonBottom && mouseY <= buttonTop) {
            if (button.action == "hit") {
                if (gameState == 2) {
                    // If the game has ended, start a new round
                    resetGame();
                    std::cout << "New round started via Hit button!" << std::endl;
                }
                else if (gameState == 1 && playerTurn) {
                    // Normal Hit functionality during gameplay
                    dealCard(playerHand);
                    int playerScore = calculateScore(playerHand);
                    if (playerScore == 21) {
                        std::cout << "Player hits 21! You win!" << std::endl;
                        gameState = 2; // End the game
                    }
                    else if (playerScore > 21) {
                        std::cout << "Player busts!" << std::endl;
                        gameState = 2; // End the game
                    }
                }
            }
            else if (button.action == "stand" && gameState == 1 && playerTurn) {
                // Stand functionality during gameplay
                playerTurn = false;
            }
            else if (button.action == "restart" && gameState != 1) {
                // Restart functionality only when the game is not in progress
                resetGame();
                std::cout << "Game restarted via Restart button!" << std::endl;
            }
        }
    }
}



void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Convert mouse coordinates to normalized device coordinates (-1 to 1)
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float normMouseX = (mouseX / width) * 2.0f - 1.0f;
        float normMouseY = 1.0f - (mouseY / height) * 2.0f;

        Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
        game->handleMouseClick(normMouseX, normMouseY);
    }
}


void Game::update() {

    glDisable(GL_DEPTH_TEST); // Ensure text appears on top
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     int dealerScore = calculateScore(dealerHand);
     int playerScore = calculateScore(playerHand);

    if (!playerTurn && gameState == 1) {
        if (dealerScore < 17) {
            dealCard(dealerHand);
        }
        else {
            std::cout << "Player Score: " << playerScore << ", Dealer Score: " << dealerScore << std::endl;
            gameState = 2; // End the game
        }
    }
    else if (gameState == 2) {
        if ((playerScore > dealerScore  && playerScore < 22)|| dealerScore > 21) {
            std::cout << "Player wins with " << playerScore << " points!" << std::endl;
            gameMessage = "PLAYER WINS!";

        }
        else if (playerScore == dealerScore) {
            std::cout << "It's a tie!" << std::endl;
            gameMessage = "IT'S A TIE!";
        }
        else if ((playerScore < dealerScore && dealerScore < 22) || playerScore > 21) {
            std::cout << "Dealer wins with " << dealerScore << " points!" << std::endl;
            gameMessage = "DEALER WINS!";
        }
        else if (playerScore > 21) {
            std::cout << "BUST!" << std::endl;
            gameMessage = "PLAYER BUSTED!";
        }
        else if (dealerScore > 21) {
            std::cout << "BUST!" << std::endl;
            gameMessage = "DEALER BUSTED!";
        }
    }

    glDisable(GL_BLEND);

    // Auto-restart when deck is empty and game ends
    if (deck.empty() && gameState == 2) {
        std::cout << "Deck is empty. Restarting the game automatically..." << std::endl;
        resetDeck();
    }
}

void Game::renderCards(const std::vector<Card>& hand, float startX, float startY, bool hideSecondCard) {
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader->getID(), "texture1"), 0);

    glBindVertexArray(VAO);
    for (size_t i = 0; i < hand.size(); ++i) {
        if (hideSecondCard && i == 1) {
            glBindTexture(GL_TEXTURE_2D, textures["cardBack"]);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, hand[i].getTextureID());
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(startX + i * 0.2f *1.3, startY, 0.0f));
        model = glm::scale(model, glm::vec3(0.18f * 1.2, 0.28f * 1.2, 1.0f));

        glUniformMatrix4fv(glGetUniformLocation(shader->getID(), "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

void Game::renderButton(float x, float y, const std::string& textureKey, const std::string& label) {
    // Render button background
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[textureKey]);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(0.4f, 0.2f, 1.0f)); // Button size

    glUniformMatrix4fv(glGetUniformLocation(shader->getID(), "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(shader->getID(), "texture1"), 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Render button label
    glDisable(GL_DEPTH_TEST); // Ensure text appears on top
    
    textShader->use();

    // Adjust text scale and alignment
    float textScale = 0.8f; // Adjust for button size
    float textWidth = label.size() * 12.0f * textScale; // Estimate text width
    float textHeight = 24.0f * textScale;              // Estimate text height
    float textX = (x + 1.0f) * (1280.0f / 2.0f) - textWidth / 2.0f; // Center horizontally
    float textY = (y + 1.0f) * (960.0f / 2.0f) - textHeight / 2.0f; // Center vertically
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    textRenderer->RenderText(*textShader, label, textX, textY, textScale, glm::vec3(1.0f, 1.0f, 1.0f));
    glDisable(GL_BLEND); // Disable blending after text rendering

    glEnable(GL_DEPTH_TEST);
}

void Game::render() {
    glClearColor(0.2f, 0.5f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing for cards and buttons
    glEnable(GL_DEPTH_TEST);

    // Render cards
    shader->use();
    renderCards(playerHand, -0.8f, 0.5f, false);

    // Hide dealer's second card during player's turn
    if (playerTurn) {
        renderCards(dealerHand, -0.8f, -0.2f, true); // Hide the second card
    }
    else {
        renderCards(dealerHand, -0.8f, -0.2f, false); // Show all cards
    }

    // Render buttons
    renderButton(-0.75f, -0.8f, "cardBack", "HIT");
    renderButton(-0.25f, -0.8f, "cardBack", "STAND");
    renderButton(0.25f, -0.8f, "cardBack", "RESTART");

    // Render text (disable depth testing and enable blending)
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Player's score
    textShader->use();
    textRenderer->RenderText(*textShader, "Player Score: " + std::to_string(calculateScore(playerHand)), 10.0f, 920.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

    // Dealer's score: Show only the first card during player's turn
    if (playerTurn) {
        textRenderer->RenderText(*textShader, "Dealer Score: " + std::to_string(calculateScore({ dealerHand.front() })), 10.0f, 880.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    else {
        textRenderer->RenderText(*textShader, "Dealer Score: " + std::to_string(calculateScore(dealerHand)), 10.0f, 880.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    if (!gameMessage.empty()) {
        textRenderer->RenderText(*textShader, gameMessage, 640.0f - (gameMessage.size() * 10.0f), 480.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    glDisable(GL_BLEND); // Disable blending after text rendering
}




void Game::run() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 960, "Blackjack", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    shader = new Shader(vertexShaderSource, fragmentShaderSource);
    textShader = new Shader(TextvertexShaderSource, TextfragmentShaderSource);

    initializeDeck();
    shuffleDeck();
    initializeCardRendering();
    loadAssets();
    resetGame();

    while (!glfwWindowShouldClose(window)) {
        handleInput(window);
        update();
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
