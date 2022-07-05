// #include "../../src/WintExp.h"
#include "armtools.h"
#include <cstdlib>
#include <cstdio>
#include <exception>
#include <sys/mman.h>

// ------------------- MEMORY -------------------------------

uintptr_t ARMTools::Memory::getLibraryPointer(const char* library) {
    char filename[0xFF] = {0},
    buffer[2048] = {0};
    FILE *fp = 0;
    uintptr_t address = 0;

    sprintf( filename, "/proc/%d/maps", getpid() );

    fp = fopen( filename, "rt" );
    if(fp == 0)
    {
        // LOG("ARMTools error code called: #%d (Library: %s, File name: %s)", arm_error_codes::cant_open_file_error, library, filename);
        goto done;
    }

    while(fgets(buffer, sizeof(buffer), fp))
    {
        if( strstr( buffer, library ) )
        {
            address = (uintptr_t)strtoul( buffer, 0, 16 );
            break;
        }
    }

    done:

    if(fp)
      fclose(fp);

    return address;
}

bool ARMTools::Memory::unprotect(uintptr_t address, size_t size) {
	bool badResult = false;
	for(uintptr_t x = address; x != address + size; x++) {
		if(!mprotect((void*)(x & 0xFFFFF000), GLOBAL_PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC)) {
			badResult = true;
		}
	}

	return !badResult;
}

void ARMTools::Memory::makeNOP(uintptr_t address, size_t size)
{
    ARMTools::Memory::unprotect(address);

    for(uintptr_t ptr = address; ptr != (address+(size*2)); ptr += 2)
    {
        *(char*)ptr = 0x00;
        *(char*)(ptr+1) = 0xBF;
    }
}

void ARMTools::Memory::writeMemory(uintptr_t dest, uintptr_t src, size_t size)
{
	ARMTools::Memory::unprotect(dest);
	memcpy((void*)dest, (void*)src, size);
	cacheflush(dest, dest+size, 0);
}

void ARMTools::Memory::readMemory(uintptr_t dest, uintptr_t src, size_t size)
{
    ARMTools::Memory::unprotect(src);
    memcpy((void*)dest, (void*)src, size);
}

void ARMTools::Memory::makeJMP(uintptr_t func, uintptr_t addr)
{
	uint32_t code = ((addr-func-4) >> 12) & 0x7FF | 0xF000 | ((((addr-func-4) >> 1) & 0x7FF | 0xB800) << 16);
    ARMTools::Memory::writeMemory(func, (uintptr_t)&code, 4);
}

void ARMTools::Memory::writeHookProc(uintptr_t addr, uintptr_t func)
{
    char code[16] = { '\0' };
    memcpy(code, "\x01\xB4\x01\xB4\x01\x48\x01\x90\x01\xBD\x00\xBF\x00\x00\x00\x00", 16);
    *(uint32_t*)&code[12] = (func | 1);
    ARMTools::Memory::writeMemory(addr, (uintptr_t)code, 16);
}


// --------------------- HOOKS ---------------------------

ARMTools::Hook::Hook(uintptr_t pointer, uintptr_t start_offset, uintptr_t end_offset) {
	this->library_pointer = pointer;
	this->trampoline_start_offset = this->library_pointer + start_offset;
	this->trampoline_end_offset = this->trampoline_start_offset + end_offset;

	this->mmap_start_offset = (uintptr_t)mmap(0, GLOBAL_PAGE_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	ARMTools::Memory::unprotect(this->mmap_start_offset);
	this->mmap_end_offset = (this->mmap_start_offset + GLOBAL_PAGE_SIZE);
}

ARMTools::Hook::~Hook() {
	this->library_pointer = 0x0;
	this->trampoline_start_offset = 0x0;
	this->trampoline_end_offset = 0x0;
	this->mmap_start_offset = 0x0;
	this->mmap_end_offset = 0x0;
}

void ARMTools::Hook::install(uintptr_t address, uintptr_t func, uintptr_t *orig, int* result)
{
	// LOG("ARMTools::Hook::install at 0x%X -> 0x%X", address, func);
	uintptr_t addr = this->library_pointer + address;

    if(this->trampoline_end_offset < (this->trampoline_start_offset + 0x10) || this->mmap_end_offset < (this->mmap_start_offset + 0x20))
    {
        // LOG("ARMTools error code called: #%d", arm_error_codes::space_limit_reached_error);
        *result = -1;
        return;
        // std::terminate();
    }

    ARMTools::Memory::readMemory(this->mmap_start_offset, addr, 4);
    ARMTools::Memory::writeHookProc(this->mmap_start_offset+4, addr+4);
    *orig = this->mmap_start_offset+1;
    this->mmap_start_offset += 32;

    ARMTools::Memory::makeJMP(addr, this->trampoline_start_offset);
    ARMTools::Memory::writeHookProc(this->trampoline_start_offset, func);
    this->trampoline_start_offset += 16;

    *result = 1;
}

void ARMTools::Hook::plt_install(uintptr_t address, uintptr_t func, uintptr_t *orig) {
	// LOG("ARMTools::Hook::plt_install | func: 0x%X -> 0x%X", address, func);
	address += this->library_pointer;
    ARMTools::Memory::unprotect(address);
    *orig = *(uintptr_t*)address;
    *(uintptr_t*)address = func;
}

void ARMTools::Hook::mtd_install(uintptr_t address, uintptr_t func) {
	// LOG("ARMTools::Hook::mtd_install | func: 0x%X -> 0x%X", address, func);
	address += this->library_pointer;
    ARMTools::Memory::unprotect(address);
    *(uintptr_t*)address = func;
}