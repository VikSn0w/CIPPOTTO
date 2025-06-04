// chip8.cpp - Improved version with bug fixes
#include "chip8.h"
#include <fstream>
#include <iostream>
#include <cstring>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int MAX_ROM_SIZE = 4096 - START_ADDRESS;

uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

chip8::chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()),
                 randByte(0, 255U) {
    program_counter = START_ADDRESS;
    opcode = 0;
    index_register = 0;
    stack_pointer = 0;
    delay_timer = 0;
    sound_timer = 0;

    resetRegistersV();
    resetMemory();

    // Load fontset into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

void chip8::LoadROM(char const* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return;
    }

    std::streampos size = file.tellg();

    if (size > MAX_ROM_SIZE) {
        std::cerr << "ROM size (" << size << " bytes) exceeds maximum allowed size ("
                  << MAX_ROM_SIZE << " bytes)" << std::endl;
        file.close();
        return;
    }

    char* buffer = new char[size];
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    // Load ROM into memory starting at 0x200
    for (long i = 0; i < size; ++i) {
        memory[START_ADDRESS + i] = static_cast<uint8_t>(buffer[i]);
    }

    delete[] buffer;
    std::cout << "Loaded ROM: " << filename << " (" << size << " bytes)" << std::endl;
}

void chip8::resetMemory() {
    std::memset(memory, 0, sizeof(memory));
}

void chip8::resetRegistersV() {
    std::memset(registers_V, 0, sizeof(registers_V));
}

void chip8::clear_display() {
    std::memset(graphics, 0, sizeof(graphics));
}

void chip8::emulateCycle() {
    // Fetch instruction
    opcode = (memory[program_counter] << 8) | memory[program_counter + 1];
    program_counter += 2;

    // Decode and execute
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: // CLS - Clear display
                    clear_display();
                    break;
                case 0x00EE: // RET - Return from subroutine
                    if (stack_pointer > 0) {
                        program_counter = stack[--stack_pointer];
                    }
                    break;
                default:
                    std::cout << "Unknown 0x0 opcode: 0x" << std::hex << opcode << std::endl;
            }
            break;

        case 0x1000: // JP addr - Jump to address
            program_counter = opcode & 0x0FFF;
            break;

        case 0x2000: // CALL addr - Call subroutine
            if (stack_pointer < 16) {
                stack[stack_pointer++] = program_counter;
                program_counter = opcode & 0x0FFF;
            }
            break;

        case 0x3000: // SE Vx, byte - Skip if Vx == byte
            if (registers_V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                program_counter += 2;
            }
            break;

        case 0x4000: // SNE Vx, byte - Skip if Vx != byte
            if (registers_V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                program_counter += 2;
            }
            break;

        case 0x5000: // SE Vx, Vy - Skip if Vx == Vy
            if (registers_V[(opcode & 0x0F00) >> 8] == registers_V[(opcode & 0x00F0) >> 4]) {
                program_counter += 2;
            }
            break;

        case 0x6000: // LD Vx, byte - Load byte into Vx
            registers_V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;

        case 0x7000: // ADD Vx, byte - Add byte to Vx
            registers_V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;

        case 0x8000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;

            switch (opcode & 0x000F) {
                case 0x0: // LD Vx, Vy
                    registers_V[x] = registers_V[y];
                    break;
                case 0x1: // OR Vx, Vy
                    registers_V[x] |= registers_V[y];
                    break;
                case 0x2: // AND Vx, Vy
                    registers_V[x] &= registers_V[y];
                    break;
                case 0x3: // XOR Vx, Vy
                    registers_V[x] ^= registers_V[y];
                    break;
                case 0x4: { // ADD Vx, Vy
                    uint16_t sum = registers_V[x] + registers_V[y];
                    registers_V[0xF] = (sum > 255) ? 1 : 0;
                    registers_V[x] = sum & 0xFF;
                    break;
                }
                case 0x5: // SUB Vx, Vy
                    registers_V[0xF] = (registers_V[x] >= registers_V[y]) ? 1 : 0;
                    registers_V[x] -= registers_V[y];
                    break;
                case 0x6: // SHR Vx
                    registers_V[0xF] = registers_V[x] & 0x1;
                    registers_V[x] >>= 1;
                    break;
                case 0x7: // SUBN Vx, Vy
                    registers_V[0xF] = (registers_V[y] >= registers_V[x]) ? 1 : 0;
                    registers_V[x] = registers_V[y] - registers_V[x];
                    break;
                case 0xE: // SHL Vx
                    registers_V[0xF] = (registers_V[x] & 0x80) >> 7;
                    registers_V[x] <<= 1;
                    break;
                default:
                    std::cout << "Unknown 0x8 opcode: 0x" << std::hex << opcode << std::endl;
            }
            break;
        }

        case 0x9000: // SNE Vx, Vy - Skip if Vx != Vy
            if (registers_V[(opcode & 0x0F00) >> 8] != registers_V[(opcode & 0x00F0) >> 4]) {
                program_counter += 2;
            }
            break;

        case 0xA000: // LD I, addr - Load address into I
            index_register = opcode & 0x0FFF;
            break;

        case 0xB000: // JP V0, addr - Jump to V0 + addr
            program_counter = (opcode & 0x0FFF) + registers_V[0];
            break;

        case 0xC000: // RND Vx, byte - Random byte AND byte
            registers_V[(opcode & 0x0F00) >> 8] = randByte(randGen) & (opcode & 0x00FF);
            break;

        case 0xD000: { // DRW Vx, Vy, nibble - Draw sprite
            uint8_t x = registers_V[(opcode & 0x0F00) >> 8];
            uint8_t y = registers_V[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0x000F;

            registers_V[0xF] = 0; // Reset collision flag

            for (uint8_t row = 0; row < height; ++row) {
                uint8_t spriteByte = memory[index_register + row];

                for (uint8_t col = 0; col < 8; ++col) {
                    if (spriteByte & (0x80 >> col)) {
                        uint16_t pixelX = (x + col) % VIDEO_WIDTH;
                        uint16_t pixelY = (y + row) % VIDEO_HEIGHT;
                        uint32_t* screenPixel = &graphics[pixelY * VIDEO_WIDTH + pixelX];

                        // Check for collision before XOR
                        if (*screenPixel != 0) {
                            registers_V[0xF] = 1;
                        }

                        // XOR the pixel (toggle it)
                        *screenPixel ^= 0xFFFFFFFF;
                    }
                }
            }
            break;
        }

        case 0xE000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x9E: // SKP Vx - Skip if key pressed
                    if (keypad[registers_V[x]]) {
                        program_counter += 2;
                    }
                    break;
                case 0xA1: // SKNP Vx - Skip if key not pressed
                    if (!keypad[registers_V[x]]) {
                        program_counter += 2;
                    }
                    break;
                default:
                    std::cout << "Unknown 0xE opcode: 0x" << std::hex << opcode << std::endl;
            }
            break;
        }

        case 0xF000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                case 0x07: // LD Vx, DT
                    registers_V[x] = delay_timer;
                    break;
                case 0x0A: { // LD Vx, K - Wait for key press
                    bool keyPressed = false;
                    for (uint8_t i = 0; i < 16; ++i) {
                        if (keypad[i]) {
                            registers_V[x] = i;
                            keyPressed = true;
                            break;
                        }
                    }
                    if (!keyPressed) {
                        program_counter -= 2; // Repeat instruction
                    }
                    break;
                }
                case 0x15: // LD DT, Vx
                    delay_timer = registers_V[x];
                    break;
                case 0x18: // LD ST, Vx
                    sound_timer = registers_V[x];
                    break;
                case 0x1E: // ADD I, Vx
                    index_register += registers_V[x];
                    break;
                case 0x29: // LD F, Vx - Load font location
                    index_register = FONTSET_START_ADDRESS + (registers_V[x] * 5);
                    break;
                case 0x33: { // LD B, Vx - Store BCD representation
                    uint8_t value = registers_V[x];
                    memory[index_register] = value / 100;
                    memory[index_register + 1] = (value / 10) % 10;
                    memory[index_register + 2] = value % 10;
                    break;
                }
                case 0x55: // LD [I], Vx - Store registers V0-Vx
                    for (int i = 0; i <= x; ++i) {
                        memory[index_register + i] = registers_V[i];
                    }
                    break;
                case 0x65: // LD Vx, [I] - Load registers V0-Vx
                    for (int i = 0; i <= x; ++i) {
                        registers_V[i] = memory[index_register + i];
                    }
                    break;
                default:
                    std::cout << "Unknown 0xF opcode: 0x" << std::hex << opcode << std::endl;
            }
            break;
        }

        default:
            std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::endl;
            break;
    }

    // Update timers
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "BEEP!" << std::endl;
        }
        --sound_timer;
    }
}