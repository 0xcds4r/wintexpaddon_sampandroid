#pragma once

#define GLOBAL_PAGE_SIZE 0x5000

#define SPL(v, c) (v + c)

#include <unistd.h>

enum arm_error_codes {
	mmap_error = 0x024,
	pointer_error = 0x025,
	hook_error = 0x026,
	page_size_error = 0x027,
	offset_error = 0x028,
	space_limit_reached_error = 0x029,
	cant_open_file_error = 0x032,
};

namespace ARMTools 
{
	class Memory;
	class Hook;	
}

class ARMTools::Memory {
public:
	static bool unprotect(uintptr_t address, size_t size = 100);
	static void makeNOP(uintptr_t address, size_t size);
	static void writeMemory(uintptr_t dest, uintptr_t src, size_t size);
	static void readMemory(uintptr_t dest, uintptr_t src, size_t size);
	static void makeJMP(uintptr_t func, uintptr_t addr);
	static void writeHookProc(uintptr_t addr, uintptr_t func);
	static uintptr_t getLibraryPointer(const char*);
};

class ARMTools::Hook
{
private:
	uintptr_t mmap_start_offset, mmap_end_offset;
	uintptr_t library_pointer, trampoline_start_offset, trampoline_end_offset;
public:
	Hook(uintptr_t pointer, uintptr_t start_offset, uintptr_t end_offset);
	~Hook();
	void install(uintptr_t address, uintptr_t func, uintptr_t *orig, int* result = 0);
	void plt_install(uintptr_t address, uintptr_t func, uintptr_t *orig);
	void mtd_install(uintptr_t address, uintptr_t func);
	
};

