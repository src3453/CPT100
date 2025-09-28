#include <vector>
#include <tuple>

std::vector<unsigned char> ram(RAM_SIZE, 0);
std::vector<unsigned char> vram(VRAM_SIZE, 0);
// RAMおよびVRAMを管理する関数
void ram_boot(std::vector<unsigned char>& ram, std::vector<unsigned char>& vram) {
    ram.resize(RAM_SIZE, 0);
    vram.resize(VRAM_SIZE, 0);
}

unsigned char vram_peek(std::vector<unsigned char>& vram, int addr) {
    if (addr < 0) {
        return 0;
    }
    if (addr < VRAM_SIZE) {
        return vram.at(addr);
    }
}

void vram_poke(std::vector<unsigned char>& vram, int addr, unsigned char val) {
    if (addr < 0) {
        return;
    }   
    if (addr < VRAM_SIZE) {
        vram.at(addr) = val;
    }
}

std::vector<unsigned char> vram_peek2array(std::vector<unsigned char>& vram, int addr, int block) {
    std::vector<unsigned char> out;
    for (int i = addr; i < addr + block; i++)
    {
        out.push_back(vram_peek(vram, i));
    }
    return out;
}

void vram_pokefill(std::vector<unsigned char>& vram, int addr, int block, unsigned char val) {
    std::fill(vram.begin() + addr, vram.begin() + addr + block, val);
}

void vram_poke2array(std::vector<unsigned char>& vram, int addr, std::vector<unsigned char>& vals) {
    std::copy(vals.begin(), vals.end(), vram.begin() + addr);
}

unsigned char ram_peek(std::vector<unsigned char>& ram, int addr) {
    if (addr < 0) {
        return 0;
    }
    if (addr < RAM_SIZE) {
        return ram.at(addr);
    }
}

void ram_poke(std::vector<unsigned char>& ram, int addr, unsigned char val) {
    if (addr < 0) {
        return;
    }   
    if (addr < RAM_SIZE) {
        ram.at(addr) = val;
    }
}

std::vector<unsigned char> ram_peek2array(std::vector<unsigned char>& ram, int addr, int block) {
    std::vector<unsigned char> out;
    for (int i = addr; i < addr + block; i++)
    {
        out.push_back(ram_peek(ram, i));
    }
    return out;
}

void ram_pokefill(std::vector<unsigned char>& ram, int addr, int block, unsigned char val) {
    std::fill(ram.begin() + addr, ram.begin() + addr + block, val);
}

void ram_poke2array(std::vector<unsigned char>& ram, int addr, std::vector<unsigned char>& vals) {
    std::copy(vals.begin(), vals.end(), ram.begin() + addr);
}

