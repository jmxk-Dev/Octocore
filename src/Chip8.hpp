#pragma once

#include <string>
#include "Common.hpp"

class Chip8 {
public:
	void Init();
	bool LoadGame(const std::string& name);
	void EmulateCycle();
	void SetKeys();
	void UpdateTimers();
	bool GetDrawFlag() const;
	void SetDrawFlag(bool value);
	void DecodeAndExecuteOpcode();

	u8 key[16]; // Stores the current state of the keys
	u8 gfx[64 * 32]; // Display as an array
private:
	u16 opcode; // Store the current opcode
	
	u8 memory[4096]; // Memory of the Chip8
	u8 V[16]; // CPU registers
	u16 I; // Index register
	u16 pc; // Program counter
	
	u16 stack[16]; // Array to save the current location before a jump
	u16 sp; // Current stack member

	// Timers
	u8 delayTimer;
	u8 soundTimer;
	bool drawFlag;
};