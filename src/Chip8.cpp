#include "Chip8.hpp"

#include <random>
#include <iostream>
#include <fstream>
#include <vector>
#include "SDL3/SDL.h"

const float TIMER = 1000.0f / 60.0f;

u8 chip8_fontset[80] = {
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

void Chip8::Init() {
    //Init registers and memory
    pc = 0x200; // Start of the program
    opcode = 0;
    I = 0;
    sp = 0;

    // Clear display
    for (int i = 0; i < 2048; i++)
        gfx[i] = 0;
    
    // Clear stack
    for (int i = 0; i < 16; i++)
        stack[i] = 0;

    // Clear registers V0-VF
    for (int i = 0; i < 16; i++)
        V[i] = 0;

    // Clear memory
    for (int i = 0; i < 4096; i++)
        memory[i] = 0;

    // Load fontset
    for (int i = 0; i < 80; i++)
        memory[i] = chip8_fontset[i];

    // Reset timers
    delayTimer = 0;
    soundTimer = 0;

    // Clear screen at the beginning
    drawFlag = true;

    srand(time(NULL));
}

bool Chip8::LoadGame(const std::string& filepath) {
    std::string fullFilepath = PROJECT_ROOT + filepath;
    // Open file
    std::ifstream file(fullFilepath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "File error\n";
        return false;
    }

    // Check file size
    std::streamsize size = file.tellg();  // get file size
    file.seekg(0, std::ios::beg);         // rewind

    std::cout << "Filesize: " << size << "\n";

    if (size > (4096 - 512)) {
        std::cerr << "Error: ROM too big for memory\n";
        return false;
    }

    std::vector<char> buffer(size);       // allocate buffer
    if (!file.read(buffer.data(), size)) { // read file into buffer
        std::cerr << "Reading error\n";
        return false;
    }

    // Copy ROM into Chip8 memory starting at 0x200 (512)
    std::copy(buffer.begin(), buffer.end(), memory + 512);


    return true;
}

void Chip8::SetKeys() {

}

void Chip8::EmulateCycle() {
    //Fetch Opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    //Decode & execute Opcode
    DecodeAndExecuteOpcode();

    UpdateTimers();
}

void Chip8::UpdateTimers() {
    if (delayTimer > 0)
        --delayTimer;
    if (soundTimer > 0)
        if (soundTimer == 1) {
            std::cout << "BEEP!\n";
            --soundTimer;
        }
    SDL_Delay(TIMER);
}

bool Chip8::GetDrawFlag() const {
    return drawFlag;
}

void Chip8::SetDrawFlag(bool value) {
    drawFlag = value;
}

void Chip8::DecodeAndExecuteOpcode() {
    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode & 0x00FF) {
        // Clears the display
        case 0x00E0:
            for (int i = 0; i < 2048; i++)
                gfx[i] = 0x0;
            drawFlag = true;
            pc += 2;
            break;
        // Return from a subroutine
        case 0x00EE:
            sp--;
            pc = stack[sp];
            pc += 2;
            break;
        }
        break;
    // Jump to location nnn
    case 0x1000:
        pc = opcode & 0x0FFF;
        break;
    // Call subroutine at nnn
    case 0x2000:
        stack[sp] = pc;
        sp++;
        pc = opcode & 0x0FFF;
        break;
    // Skip next instruction if Vx == kk
    case 0x3000:
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;
    // Skip next instruction if Vx != kk
    case 0x4000:
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;
    // Skip next instruction if Vx == Vy
    case 0x5000:
        if (V[(opcode & 0x0F00) >> 8] == V[opcode & 0x00F0] >> 4)
            pc += 4;
        else
            pc += 2;
        break;
    // Set Vx == kk
    case 0x6000:
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;
    // Set Vx = Vx + kk
    case 0x7000:
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;
    case 0x8000:
        switch (opcode & 0x000F) {
        // Set Vx = Vy
        case 0x0000:
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        // Set Vx = Vx OR Vy
        case 0x0001:
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        // Set Vx = Vx AND Vy
        case 0x0002:
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        // Set Vx = Vx XOR Vy
        case 0x0003:
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        // Set Vx = Vx + Vy, set VF = carry
        case 0x0004:
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        // Set Vx = Vx - Vy, set VF = NOT borrow
        case 0x0005:
            if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        // Set Vx = Vx SHR 1
        case 0x0006:
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;
        // Set Vx = Vy - Vx, set VF = NOT borrow
        case 0x0007:
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        // Set Vx = Vx SHL 1
        case 0x000E:
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;
        }
        break;
    // Skip next instruction if Vx != Vy
    case 0x9000:
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;
    // Set I = nnn
    case 0xA000:
        I = opcode & 0x0FFF;
        pc += 2;
        break;
    // Jump to location nnn + V0
    case 0xB000:
        pc = (opcode & 0x0FFF) + V[0];
        break;
    // Set Vx = random byte AND kk
    case 0xC000:
        V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
        pc += 2;
        break;
    // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
    case 0xD000:
        {
            u16 x = V[(opcode & 0x0F00) >> 8];
            u16 y = V[(opcode & 0x00F0) >> 4];
            u16 height = opcode & 0x000F;
            u16 pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++)
                {
                    if ((pixel & (0x80 >> xline)) != 0)
                    {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pc += 2;
        }
        break;
    case 0xE000:
        switch (opcode & 0x00FF) {
        // Skip next instruction if key with the value of Vx is pressed
        case 0x009E:
            if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                pc += 4;
            else
                pc += 2;
            break;
        // Skip next instruction if key with the value of Vx is not pressed
        case 0x00A1:
            if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                pc += 4;
            else
                pc += 2;
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF) {
        // Set Vx = delay timer value
        case 0x0007:
            V[(opcode & 0x0F00) >> 8] = delayTimer;
            pc += 2;
            break;
        // Wait for a key press, store the value of the key in Vx
        case 0x0A:
            {
                bool keyPress = false;

                for (int i = 0; i < 16; ++i)
                {
                    if (key[i] != 0)
                    {
                        V[(opcode & 0x0F00) >> 8] = i;
                        keyPress = true;
                    }
                }

                if (!keyPress)
                    return;

                pc += 2;
            }
            break;
        // Set delay timer = Vx
        case 0x0015:
            delayTimer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        // Set sound timer = Vx
        case 0x0018:
            soundTimer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        // Set I = I + Vx
        case 0x001E:
            if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                V[0xF] = 1;
            else
                V[0xF] = 0;
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        // Set I = location of sprite for digit Vx
        case 0x0029:
            I = V[(opcode & 0x0F00) >> 8] * 0x5;
            pc += 2;
            break;
        // Store BCD representation of Vx in memory locations I, I+1, and I+2
        case 0x0033:
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;
        // Store registers V0 through Vx in memory starting at location I
        case 0x0055:
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                memory[I + i] = V[i];

            // On the original interpreter, when the operation is done, I = I + X + 1.
            I += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;
            break;
        // Read registers V0 through Vx from memory starting at location I
        case 0x0065:
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                V[i] = memory[I + i];

            // On the original interpreter, when the operation is done, I = I + X + 1.
            I += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;
            break;
        }
        break;
    default:
        std::cout << "Unknown opcode: 0x%X" << std::hex << opcode << "\n";
        break;
    }
}