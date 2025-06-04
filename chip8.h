//
// Created by vit-pic on 5/31/25.
//

#ifndef CHIP8_H
#define CHIP8_H
#include <cstdint>
#include <chrono>
#include <random>

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class chip8 {
    public:
        uint8_t registers_V[16]{};
        uint8_t memory[4096]{};
        uint32_t graphics[64*32]{};
        uint8_t keypad[16]{};

        uint8_t delay_timer{};
        uint8_t sound_timer{};

        uint16_t stack_pointer{};
        uint16_t stack[16]{};
        uint16_t index_register{};
        uint16_t program_counter{};
        uint16_t opcode{};

        std::default_random_engine randGen;
        std::uniform_int_distribution<uint8_t> randByte;

        chip8();

        void LoadROM(char const *filename);

        void resetMemory();

        void resetRegistersV();

        void clear_display();


        void emulateCycle();
};



#endif //CHIP8_H
