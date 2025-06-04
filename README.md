# CHIP-8 Emulator

A cross-platform CHIP-8 emulator implementation written in C++ with command-line interface and optional debug windows.

## Features

- **Complete CHIP-8 instruction set implementation**
- **Cross-platform support** (Windows and Linux)
- **Command-line interface** with ASCII graphics display
- **Real-time debug console** (Windows only) showing:
  - CPU registers and timers
  - Memory visualization around PC
  - Stack contents
  - Keypad state
- **Configurable emulation speed**
- **Standard CHIP-8 keypad mapping**

## Building

### Prerequisites

- C++11 compatible compiler (GCC, Clang, or MSVC)
- CMake (optional, for build system)
- SFML library (for window management, though currently using console output)

### Compilation

```bash
# Using g++
g++ -std=c++11 -o chip8 main.cpp chip8.cpp PlatformConsole.cpp DebugConsoleManager.cpp

# Or with CMake (if CMakeLists.txt is available)
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
./chip8 <Scale> <Delay> <ROM> [debug]
```

### Parameters

- **Scale**: Display scale factor (kept for compatibility, not used in console version)
- **Delay**: Cycle delay in milliseconds (recommended: 1-10)
- **ROM**: Path to the CHIP-8 ROM file
- **debug**: Optional parameter to enable debug windows (Windows only)

### Examples

```bash
# Run a ROM with 2ms delay
./chip8 10 2 games/pong.ch8

# Run with debug console enabled
./chip8 10 2 games/tetris.ch8 debug
```

## Controls

The CHIP-8 keypad is mapped to your keyboard as follows:

```
CHIP-8 Keypad    Keyboard
┌─────────────┐  ┌─────────────┐
│ 1 2 3 C     │  │ 1 2 3 4     │
│ 4 5 6 D     │  │ Q W E R     │
│ 7 8 9 E     │  │ A S D F     │
│ A 0 B F     │  │ Z X C V     │
└─────────────┘  └─────────────┘
```

- **ESC** or **Q**: Quit the emulator

## Debug Features

When running with the `debug` parameter on Windows, a separate debug console window will open showing:

### Registers Section
- Program Counter (PC)
- Index Register (I)
- Stack Pointer (SP)
- Current Opcode
- All 16 V registers (V0-VF)
- Delay and Sound timers

### Memory Section
- Memory contents around the current Program Counter
- Hexadecimal display with current PC highlighted

### Stack Section
- Current stack contents
- Stack pointer position
- Call stack visualization

### Keypad Section
- Real-time keypad state
- Visual representation of pressed keys

## Project Structure

```
┌── main.cpp                 # Main application entry point
├── chip8.h                  # CHIP-8 emulator class header
├── chip8.cpp                # CHIP-8 emulator implementation
├── PlatformConsole.h        # Cross-platform console interface header
├── PlatformConsole.cpp      # Console display and input handling
├── Platform.h               # Cross-platform grahical SFML interface header (not working)
├── Platform.cpp             # Grahical display and input handling (not working)
├── DebugConsoleManager.h    # Debug console manager header
├── DebugConsoleManager.cpp  # Debug console implementation (Windows)
├── chip8-test-rom.ch8       # Demo ROM
└── README.md                # This file
```

## CHIP-8 Specifications

### Memory Layout
- **0x000-0x1FF**: CHIP-8 interpreter (contains font set in emulator)
- **0x050-0x0A0**: Used for the built-in 4x5 pixel font set (0-F)
- **0x200-0xFFF**: Program ROM and work RAM

### Registers
- **16 8-bit data registers** (V0-VF)
- **16-bit address register** (I)
- **Program counter** (PC)
- **8-bit delay timer**
- **8-bit sound timer**
- **Stack** (16 levels of 16-bit values)

### Display
- **64x32 pixel monochrome display**
- **Sprites** are 8 pixels wide and 1-15 pixels high
- **XOR drawing** (pixels are flipped)

## Supported Instructions

The emulator implements all 35 standard CHIP-8 instructions:

- **0nnn**: SYS addr (ignored in modern implementations)
- **00E0**: CLS - Clear display
- **00EE**: RET - Return from subroutine
- **1nnn**: JP addr - Jump to address
- **2nnn**: CALL addr - Call subroutine
- **3xkk**: SE Vx, byte - Skip if equal
- **4xkk**: SNE Vx, byte - Skip if not equal
- **5xy0**: SE Vx, Vy - Skip if registers equal
- **6xkk**: LD Vx, byte - Load byte into register
- **7xkk**: ADD Vx, byte - Add byte to register
- **8xy0-8xyE**: Various ALU operations
- **9xy0**: SNE Vx, Vy - Skip if registers not equal
- **Annn**: LD I, addr - Load address into I
- **Bnnn**: JP V0, addr - Jump to V0 + address
- **Cxkk**: RND Vx, byte - Random byte AND kk
- **Dxyn**: DRW Vx, Vy, nibble - Draw sprite
- **Ex9E/ExA1**: Key input instructions
- **Fx07-Fx65**: Various utility instructions

## Known Limitations

- Sound output is limited to console "BEEP!" messages
- Debug console is Windows-only
- Graphics are displayed using ASCII characters in the terminal
- Some games may require specific timing adjustments

## ROMs

The emulator can run standard CHIP-8 ROM files. The included demo is [**chip8-test-rom**](https://github.com/corax89/chip8-test-rom), made by corax89. ![image](https://github.com/user-attachments/assets/63117399-00ea-4bcb-92a3-f44734b5ebc4)


ROMs should be in binary format with the `.ch8` extension.

## Troubleshooting

### Common Issues

1. **ROM won't load**: Check file path and ensure ROM file exists
2. **Graphics not displaying**: Verify terminal supports extended ASCII characters
3. **Debug console not opening**: Ensure you're running on Windows with the `debug` parameter
4. **Game running too fast/slow**: Adjust the delay parameter (1-10ms recommended)

### Platform-Specific Notes

#### Windows
- Debug console uses Windows Console API
- Supports CP437 character encoding for box drawing
- Console colors and positioning are automatically managed

#### Linux/Unix
- Uses ANSI escape sequences for display
- Terminal must support extended ASCII for proper graphics display
- Debug console features are not available

## Contributing

Feel free to contribute improvements, bug fixes, or additional features:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source. Please check the repository for specific license terms.

## References

- [CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [CHIP-8 Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

---

**Note**: This emulator is designed for educational purposes and retro gaming enthusiasts. It aims to provide an accurate implementation of the original CHIP-8 virtual machine.
