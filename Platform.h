// Platform.h - Fixed version for SFML 3
#ifndef PLATFORM_H
#define PLATFORM_H

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>
#include <iostream>

class Platform {
public:
    Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
        : textureWidth(textureWidth), textureHeight(textureHeight),
          window(sf::VideoMode(sf::Vector2u(static_cast<unsigned int>(windowWidth), static_cast<unsigned int>(windowHeight))), title),
          sprite(texture),
          pixelBuffer(textureWidth * textureHeight * 4, 0) // RGBA format
    {
        // Create texture with proper size (SFML 3 syntax)
        if (!texture.resize(sf::Vector2u(static_cast<unsigned int>(textureWidth), static_cast<unsigned int>(textureHeight)))) {
            std::cerr << "Failed to create texture!" << std::endl;
        }

        // Scale sprite to fill window if dimensions differ
        float scaleX = static_cast<float>(windowWidth) / textureWidth;
        float scaleY = static_cast<float>(windowHeight) / textureHeight;
        sprite.setScale(sf::Vector2f(scaleX, scaleY));

        // Initialize pixel buffer with black pixels (RGBA format)
        for (size_t i = 0; i < pixelBuffer.size(); i += 4) {
            pixelBuffer[i] = 0;     // Red
            pixelBuffer[i + 1] = 0; // Green
            pixelBuffer[i + 2] = 0; // Blue
            pixelBuffer[i + 3] = 255; // Alpha (fully opaque)
        }

        // Update texture with initial black screen
        texture.update(pixelBuffer.data());
    }

    ~Platform() {
        // SFML handles cleanup automatically
    }

    void Update(void const* buffer, int pitch) {
        // Convert from CHIP-8 format to SFML-compatible format
        const uint32_t* pixels = static_cast<const uint32_t*>(buffer);

        // Debug: Count pixels and check conversion
        static int frameCount = 0;
        int whitePixels = 0;

        // Create RGBA pixel array - try a different approach
        for (int y = 0; y < textureHeight; ++y) {
            for (int x = 0; x < textureWidth; ++x) {
                int pixelIndex = y * textureWidth + x;
                int bufferIndex = pixelIndex * 4;

                // Check if pixel is "on" (white) or "off" (black)
                if (pixels[pixelIndex] != 0) {
                    // White pixel
                    pixelBuffer[bufferIndex + 0] = 255; // Red
                    pixelBuffer[bufferIndex + 1] = 255; // Green
                    pixelBuffer[bufferIndex + 2] = 255; // Blue
                    pixelBuffer[bufferIndex + 3] = 255; // Alpha
                    whitePixels++;
                } else {
                    // Black pixel
                    pixelBuffer[bufferIndex + 0] = 0;   // Red
                    pixelBuffer[bufferIndex + 1] = 0;   // Green
                    pixelBuffer[bufferIndex + 2] = 0;   // Blue
                    pixelBuffer[bufferIndex + 3] = 255; // Alpha
                }
            }
        }

        // Try multiple texture update methods
        sf::Image tempImage(sf::Vector2u(static_cast<unsigned int>(textureWidth), static_cast<unsigned int>(textureHeight)), pixelBuffer.data());

        // Method 1: loadFromImage (what your original code used)
        if (!texture.loadFromImage(tempImage)) {
            std::cout << "Method 1 failed!" << std::endl;
        }

        // Debug output every 120 frames (2 seconds at 60fps)
        if (frameCount % 120 == 0) {
            std::cout << "Frame " << frameCount << " - White pixels: " << whitePixels << std::endl;

            if (whitePixels > 0) {
                // Let's examine what the first few pixels actually contain
                std::cout << "First 10 pixels from CHIP-8: ";
                for (int i = 0; i < 10; ++i) {
                    std::cout << std::hex << pixels[i] << " ";
                }
                std::cout << std::dec << std::endl;

                // And what we converted them to
                std::cout << "First 10 RGBA pixels: ";
                for (int i = 0; i < 10; ++i) {
                    int idx = i * 4;
                    std::cout << "(" << (int)pixelBuffer[idx] << "," << (int)pixelBuffer[idx+1]
                              << "," << (int)pixelBuffer[idx+2] << "," << (int)pixelBuffer[idx+3] << ") ";
                }
                std::cout << std::endl;

                // Try to manually force some white pixels for testing
                std::cout << "Testing: forcing some pixels to white..." << std::endl;
                for (int i = 0; i < 100; ++i) { // First 100 pixels
                    int idx = i * 4;
                    pixelBuffer[idx + 0] = 255; // Red
                    pixelBuffer[idx + 1] = 255; // Green
                    pixelBuffer[idx + 2] = 255; // Blue
                    pixelBuffer[idx + 3] = 255; // Alpha
                }

                // Update texture again with forced pixels
                sf::Image testImage(sf::Vector2u(static_cast<unsigned int>(textureWidth), static_cast<unsigned int>(textureHeight)), pixelBuffer.data());
                texture.loadFromImage(testImage);
            } else {
                std::cout << "No white pixels detected" << std::endl;
            }
        }

        frameCount++;

        // Normal rendering
        window.clear(sf::Color(50, 50, 50)); // Dark gray background
        window.draw(sprite);
        window.display();
    }

    bool ProcessInput(uint8_t* keys) {
        bool quit = false;

        // Clear all keys first
        for (int i = 0; i < 16; ++i) {
            keys[i] = 0;
        }

        // Process events
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                quit = true;
            }
            // Handle key press events for better responsiveness
            else if (event->is<sf::Event::KeyPressed>()) {
                auto keyPressed = event->getIf<sf::Event::KeyPressed>();
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    quit = true;
                }
            }
        }

        // Check current key states for CHIP-8 keypad
        // CHIP-8 Keypad layout:
        // 1 2 3 C
        // 4 5 6 D
        // 7 8 9 E
        // A 0 B F

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) {
            keys[0] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
            keys[1] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
            keys[2] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) {
            keys[3] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
            keys[4] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            keys[5] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
            keys[6] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            keys[7] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            keys[8] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            keys[9] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) {
            keys[0xA] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
            keys[0xB] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4)) {
            keys[0xC] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            keys[0xD] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
            keys[0xE] = 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::V)) {
            keys[0xF] = 1;
        }

        return quit;
    }

private:
    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;
    int textureWidth;
    int textureHeight;
    std::vector<uint8_t> pixelBuffer; // RGBA pixel buffer
};

#endif //PLATFORM_H