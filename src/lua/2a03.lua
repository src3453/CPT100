-- HLE NES emulator main module

-- Constants
MASTER_CLOCK_HZ = 21477272 -- NTSC NES master clock frequency (21.477272 MHz)
CPU_CLOCK_HZ = MASTER_CLOCK_HZ / 12 -- divided by 12 for NTSC NES (1.789773 MHz)
FRAME_RATE = 60.0988 -- NTSC NES frame rate (60.0988 Hz)
PPU_HEIGHT = 240 -- PPU vertical resolution
PPU_WIDTH = 256 -- PPU horizontal resolution
WRAM_SIZE = 2048 -- 2KB internal RAM size


CPU = {} -- CPU module (Ricoh RP2A03, which is compatible with MOS 6502)
PPU = {} -- PPU module (Ricoh RP2C02)
APU = {} -- APU module (pAPU, built into Ricoh RP2A03)
Cart = {} -- Cartridge module (game ROM and mapper, iNES format parser)

function Cart:init()
    
end

function Cart:load(filename)

end

function CPU:init()
    self.PC = 0x0000 -- Program Counter (will be set to reset vector on reset ($FFFC))
    self.SP = 0xFD -- Stack Pointer (initialized to $FD on reset)
    self.A = 0x00 -- Accumulator
    self.X = 0x00 -- X Register
    self.Y = 0x00 -- Y Register
    self.P = 0x24 -- Processor Status (00100100, NV-BDIZC, with unused bit set and I flag set)
end

function CPU:cycle()
    
end

function PPU:init()
    
end

function PPU:cycle()
    
end

function APU:init()
    self.frameCounter = 0 -- Frame counter for audio timing
    self.frameCounterLFSR = 0 -- Frame counter LFSR for noise generation
    self.channels = {
        pulse1 = {
            duty = 0, -- Duty cycle (2bit)
            lc_halt = false, -- Length counter halt flag (1 = halt, 0 = normal)
            constant_volume = false, -- Constant volume flags (1 = use constant volume, 0 = use envelope)
            volume = 0, -- Volume or envelope divider period (4bit)            
            timer = 0, -- Timer value (11bit)
            length_counter = 0, -- Length counter value
            sweep = {
                enabled = false, -- Sweep enable flag
                period = 0, -- Sweep period (3bit)
                negate = false, -- Sweep negate flag (1 = negate, 0 = normal)
                shift = 0, -- Sweep shift count (3bit)
            }
        }, -- Pulse channel 1
        pulse2 = {
            duty = 0, -- Duty cycle (2bit)
            lc_halt = false, -- Length counter halt flag (1 = halt, 0 = normal)
            constant_volume = false, -- Constant volume flags (1 = use constant volume, 0 = use envelope)
            volume = 0, -- Volume or envelope divider period (4bit)
            timer = 0, -- Timer value (11bit)
            length_counter = 0, -- Length counter value
            sweep = {
                enabled = false, -- Sweep enable flag
                period = 0, -- Sweep period (3bit)
                negate = false, -- Sweep negate flag (1 = negate, 0 = normal)
                shift = 0, -- Sweep shift count (3bit)
            }
        }, -- Pulse channel 2
        triangle = {
            control = false, -- control flag (1 = halt length counter and linear counter)
            linear_counter = 0, -- Linear counter reload value (7bit)
            timer = 0, -- Timer value (11bit)
            length_counter = 0 -- Length counter value
        }, -- Triangle channel
        noise = {
            lc_halt = false, -- Length counter halt flag (1 = halt, 0 = normal)
            constant_volume = false, -- Constant volume flags (1 = use constant volume, 0 = use envelope)
            volume = 0, -- Volume or envelope divider period (4bit)
            noise_mode = false, -- Noise mode (0 = normal, 1 = looped (cannot be used in earlier Famicom))
            noise_period = 0, -- Noise period (4bit)
            length_counter = 0 -- Length counter value
        }, -- Noise channel
        dmc = {
            irq_enabled = false, -- IRQ enabled flag
            loop = false, -- Loop flag
            rate = 0, -- Rate index (4bit)
            direct_load = 0, -- Direct load value (7bit)
            sample_address = 0, -- Sample address (8bit)
            sample_length = 0 -- Sample length (8bit)
        } -- DMC (DPCM) channel
    }
    
end

function APU:cycle()
    
end