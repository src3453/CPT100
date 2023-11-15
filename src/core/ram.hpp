#include <vector>


class Lib {
public:
    std::vector<Byte> ram;
    std::vector<Byte> vram;
    Lib() {
        ram.resize(RAM_SIZE, 0);
        vram.resize(VRAM_SIZE, 0);
    }

    void boot() {
        for (int i = 0; i < RAM_SIZE; ++i) {
            ram[i] = 0;
        }
        for (int i = 0; i < VRAM_SIZE; ++i) {
            vram[i] = 0;
        }
    }

    void lib_poke(int addr, Byte val, int domain) {
        if (domain == 0) {
            ram[addr] = val;
        } else {
            vram[addr] = val;
        }
    }

    Byte lib_peek(int addr, int domain) {
        if (domain == 0) {
            return ram[addr];
        } else {
            return vram[addr];
        }
    }
};

class Vram {
public:
    std::vector<Byte>& vram;
    Lib& ram;
    Vram(Lib& r) : vram(r.vram), ram(r) {}

    Byte peek(int addr) {
        return ram.lib_peek(addr, 1);
    }

    void poke(int addr, Byte val) {
        ram.lib_poke(addr, val, 1);
    }

    std::vector<Byte> peek2array(int addr, int block) {
        return std::vector<Byte>(vram.begin() + addr, vram.begin() + addr + block);
    }

    void pokefill(int addr, int block, Byte val) {
        std::fill(vram.begin() + addr, vram.begin() + addr + block, val);
    }

    void poke2array(int addr, std::vector<Byte>& vals) {
        std::copy(vals.begin(), vals.end(), vram.begin() + addr);
    }
};

class Ram {
public:
    std::vector<Byte>& RAM;
    Lib& ram;
    Ram(Lib& r) : RAM(r.ram), ram(r) {}

    Byte peek(int addr) {
        return ram.lib_peek(addr, 1);
    }

    void poke(int addr, Byte val) {
        ram.lib_poke(addr, val, 1);
    }

    std::vector<Byte> peek2array(int addr, int block) {
        return std::vector<Byte>(RAM.begin() + addr, RAM.begin() + addr + block);
    }

    void pokefill(int addr, int block, Byte val) {
        std::fill(RAM.begin() + addr, RAM.begin() + addr + block, val);
    }

    void poke2array(int addr, std::vector<Byte>& vals) {
        std::copy(vals.begin(), vals.end(), RAM.begin() + addr);
    }
};

// 同様の手順で ram クラスも移植する
// ram クラスの移植には、Lib クラスを利用することになる
