-- CHIP-8 Emulator
-- This emulator is based on the most common specification of CHIP-8.
-- 64x32 monochrome display, 16 8-bit registers, 4K memory, stack for subroutines, hex keypad.
-- keymap:
-- 1 2 3 4       -> 1 2 3 C
-- Q W E R       -> 4 5 6 D
-- A S D F       -> 7 8 9 E
-- Z X C V       -> A 0 B F

local c8 = {}
c8.memory = {}
c8.V = {}
c8.I = 0
c8.pc = 0x200
c8.stack = {}
c8.sp = 0
c8.delay_timer = 0
c8.sound_timer = 0
c8.display = {}  -- Plane 0
c8.display2 = {} -- Plane 1 (XO-CHIP)
c8.keys = {}
c8.wait_for_key = nil
c8.draw_flag = false
c8.step = 10 -- number of instructions to execute per frame (try changing this value for speed)
c8.extended_mode = false -- Super-CHIP extended mode (128x64) vs standard (64x32)
c8.plane = 1 -- XO-CHIP plane selection (1=plane0, 2=plane1, 3=both)
c8.pitch = 64 -- XO-CHIP pitch register (4000*2^((pitch-64)/48) Hz)
c8.audio_buffer = {} -- XO-CHIP 16-byte audio pattern buffer
c8.pending_long_i = nil -- Pending long I register load
c8.flag_registers = {} -- XO-CHIP flag registers (16 bytes)

--[[_poke=poke
function poke(addr,val)
    _poke(addr,val)
    txt = ""
    for addr = 0x0000, 0x003f do
        txt = txt .. string.format("%02X ", peek(addr)) .. " "
        if (addr - 0x0000 + 1) % 16 == 0 then
            txt = txt .. "\n"
        end
    end
    trace(txt)
end]]

local band = bit.band
local bor = bit.bor
local bxor = bit.bxor
local rshift = bit.rshift
local lshift = bit.lshift

-- Initialize display for standard mode (64x32)
for i = 0, 127 do
    c8.display[i] = {}
    c8.display2[i] = {}
    for j = 0, 63 do
        c8.display[i][j] = 0
        c8.display2[i][j] = 0
    end
end

-- Initialize flag registers
for i = 0, 15 do
    c8.flag_registers[i] = 0
end

-- Initialize audio buffer
for i = 0, 15 do
    c8.audio_buffer[i] = 0
end

-- XO-CHIP Audio pattern playback with PCM
function setup_pcm_audio()
    -- Use channel 9 (CH9) at 0x400200 for PCM playback
    -- Set mode to PCM waveform mode
    poke(0x400203, 0x00)  -- Mode: PCM waveform
    -- Load audio pattern into memory at 0x000100 (after fonts)
    -- and set PCM start address
    local addr_start = 0x100
    for i = 0, 127 do
        local bit = band(rshift(c8.audio_buffer[math.floor(i / 8 + 1)] or 0, (i % 8)), 0x1)
        poke(0x0 + i, bit * 255)  -- Copy pattern buffer to PCM waveform area
    end
end

function beep(freq)
    -- XO-CHIP: Use PCM audio playback with pitch control
    -- Frequency formula: freq_hz = 4000 * 2^((pitch - 64) / 48)
    -- We need to set the pitch register first, then trigger audio
    if freq == 0 then
        -- Stop buzzer
        poke(0x400202, 0)  -- Stop CH9
        return
    end
    -- For compatibility with old beep calls, use a simple pitch
    -- Map freq to pitch value approximately
    -- If freq = 1000, use pitch = 64 (4000 Hz base)
    local pitch = c8.pitch
    local freq_hz = (4000 * (2 ^ ((pitch - 64) / 48)) ) / 32
    poke(0x400200, math.floor(freq_hz/256))  -- freq high
    --poke(0x400200, 10) -- temporal value to trigger sound
    poke(0x400201, math.floor(freq_hz%256)) -- freq low
    -- Trigger PCM playback
    poke(0x400203, 0x00)  -- Mode: PCM
    poke(0x400202, 0xFF)  -- Vol: full volume
    poke(0x400208, 0xFF)  -- L/R Vol: full volume
    poke(0x400210, 0x00)  -- Start address high byte
    poke(0x400211, 0x00)  -- Start address middle byte
    poke(0x400212, 0x00)  -- Start address low byte (0x000000)
    poke(0x400213, 0x00)  -- End high byte
    poke(0x400214, 0x00)  -- End middle byte
    poke(0x400215, 0x80)  -- End low byte (128 bytes)
    poke(0x400216, 0x00)  -- Loop start high byte
    poke(0x400217, 0x00)  -- Loop start middle byte
    poke(0x400218, 0x00)  -- Loop start low byte (0x000000)
    wtSync(0)  -- Gate: start playing

end

-- Initialize PCM at startup
function init_xochip_audio()
    -- Initialize audio pattern buffer with a simple square wave
    -- 8 bytes on, 8 bytes off
    for i = 0, 7 do
        c8.audio_buffer[i*2] = 0xFF
    end
    setup_pcm_audio()
end

function BOOT()
    screen(0)
    c8.memory = {}
    for i = 0, 4095 do
        c8.memory[i] = 0
    end
    c8.V = {}
    for i = 0, 15 do
        c8.V[i] = 0
    end
    c8.I = 0
    c8.pc = 0x200
    c8.stack = {}
    c8.sp = 0
    c8.delay_timer = 0
    c8.sound_timer = 0
    c8.keys = {}
    for i = 0, 15 do
        c8.keys[i] = 0
    end
    c8.wait_for_key = nil
    c8.draw_flag = false
    c8.extended_mode = false
    c8.plane = 1
    c8.pitch = 64
    c8.pending_long_i = nil
    
    -- fontset (0-F) - 5 bytes each, stored at 0x000-0x04F
    local font = {
        0xF0,0x90,0x90,0x90,0xF0, -- 0
        0x20,0x60,0x20,0x20,0x70, -- 1
        0xF0,0x10,0xF0,0x80,0xF0, -- 2
        0xF0,0x10,0xF0,0x10,0xF0, -- 3
        0x90,0x90,0xF0,0x10,0x10, -- 4
        0xF0,0x80,0xF0,0x10,0xF0, -- 5
        0xF0,0x80,0xF0,0x90,0xF0, -- 6
        0xF0,0x10,0x20,0x40,0x40, -- 7
        0xF0,0x90,0xF0,0x90,0xF0, -- 8
        0xF0,0x90,0xF0,0x10,0xF0, -- 9
        0xF0,0x90,0xF0,0x90,0x90, -- A
        0xE0,0x90,0xE0,0x90,0xE0, -- B
        0xF0,0x80,0x80,0x80,0xF0, -- C
        0xE0,0x90,0x90,0x90,0xE0, -- D
        0xF0,0x80,0xF0,0x80,0xF0, -- E
        0xF0,0x80,0xF0,0x80,0x80  -- F
    }
    for i = 0, #font - 1 do
        c8.memory[i] = font[i + 1]
    end

    -- Extended font for Super-CHIP (0-9) - 10 bytes each, stored at 0x050-0x0A3
    -- Large 8x10 font for digits
    local ext_font = {
        -- 0
        0x3C,0x7E,0xE7,0xC3,0xC3,0xC3,0xC3,0xE7,0x7E,0x3C,
        -- 1
        0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x7E,0xFF,
        -- 2
        0x7E,0xFF,0xC3,0x03,0x0E,0x3C,0x70,0xE0,0xFF,0xFF,
        -- 3
        0x7E,0xFF,0xC3,0x03,0x1E,0x1E,0x03,0xC3,0xFF,0x7E,
        -- 4
        0x0E,0x1E,0x3E,0x76,0xE6,0xFF,0xFF,0x06,0x06,0x06,
        -- 5
        0xFF,0xFF,0xE0,0xE0,0xFE,0xFF,0x03,0xC3,0xFF,0x7E,
        -- 6
        0x3E,0x7E,0xE0,0xE0,0xFE,0xFF,0xC3,0xC3,0xFF,0x7E,
        -- 7
        0xFF,0xFF,0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0xC0,
        -- 8
        0x7E,0xFF,0xC3,0xC3,0x7E,0x7E,0xC3,0xC3,0xFF,0x7E,
        -- 9
        0x7E,0xFF,0xC3,0xC3,0xFF,0x7F,0x03,0x03,0x7E,0x3E
    }
    for i = 0, #ext_font - 1 do
        c8.memory[0x50 + i] = ext_font[i + 1]
    end

    -- load ROM
    local f = io.open("rom.ch8", "rb")
    if f then
        local data = f:read("*a")
        f:close()
        if data then
            for i = 1, #data do
                c8.memory[0x200 + (i - 1)] = string.byte(data, i)
            end
        end
    end
    
    -- Initialize XO-CHIP audio
    init_xochip_audio()
end

-- wrapper to draw a pixel on the screen
function draw(virtual_x,virtual_y,bit)
    local SCALE = 3
    if c8.extended_mode then
        SCALE = 3  -- Smaller scale for extended mode (128x64)
    else
        SCALE = 6  -- Standard scale for normal mode (64x32)
    end
    if bit == 1 then
        rect(virtual_x*SCALE,virtual_y*SCALE,SCALE,SCALE,255)
    end
end

-- Helper function to scroll display down
function scroll_down(n)
    local height = c8.extended_mode and 64 or 32
    local width = c8.extended_mode and 128 or 64
    -- Shift rows down by n pixels
    for _ = 1, n do
        for i = 0, width - 1 do
            for j = height - 1, 1, -1 do
                if band(c8.plane, 1) ~= 0 then
                    c8.display[i][j] = c8.display[i][j - 1]
                end
                if band(c8.plane, 2) ~= 0 then
                    c8.display2[i][j] = c8.display2[i][j - 1]
                end
            end
            if band(c8.plane, 1) ~= 0 then
                c8.display[i][0] = 0
            end
            if band(c8.plane, 2) ~= 0 then
                c8.display2[i][0] = 0
            end
        end
    end
    c8.draw_flag = true
end

-- Helper function to scroll display up (XO-CHIP)
function scroll_up(n)
    local height = c8.extended_mode and 64 or 32
    local width = c8.extended_mode and 128 or 64
    -- Shift rows up by n pixels
    for _ = 1, n do
        for i = 0, width - 1 do
            for j = 0, height - 2 do
                if band(c8.plane, 1) ~= 0 then
                    c8.display[i][j] = c8.display[i][j + 1]
                end
                if band(c8.plane, 2) ~= 0 then
                    c8.display2[i][j] = c8.display2[i][j + 1]
                end
            end
            if band(c8.plane, 1) ~= 0 then
                c8.display[i][height - 1] = 0
            end
            if band(c8.plane, 2) ~= 0 then
                c8.display2[i][height - 1] = 0
            end
        end
    end
    c8.draw_flag = true
end

-- Helper function to scroll display left
function scroll_left()
    local height = c8.extended_mode and 64 or 32
    local width = c8.extended_mode and 128 or 64
    -- Shift columns left by 4 pixels
    for i = 0, width - 5 do
        for j = 0, height - 1 do
            if band(c8.plane, 1) ~= 0 then
                c8.display[i][j] = c8.display[i + 4][j]
            end
            if band(c8.plane, 2) ~= 0 then
                c8.display2[i][j] = c8.display2[i + 4][j]
            end
        end
    end
    for i = width - 4, width - 1 do
        for j = 0, height - 1 do
            if band(c8.plane, 1) ~= 0 then
                c8.display[i][j] = 0
            end
            if band(c8.plane, 2) ~= 0 then
                c8.display2[i][j] = 0
            end
        end
    end
    c8.draw_flag = true
end

-- Helper function to scroll display right
function scroll_right()
    local height = c8.extended_mode and 64 or 32
    local width = c8.extended_mode and 128 or 64
    -- Shift columns right by 4 pixels
    for i = width - 1, 4, -1 do
        for j = 0, height - 1 do
            if band(c8.plane, 1) ~= 0 then
                c8.display[i][j] = c8.display[i - 4][j]
            end
            if band(c8.plane, 2) ~= 0 then
                c8.display2[i][j] = c8.display2[i - 4][j]
            end
        end
    end
    for i = 0, 3 do
        for j = 0, height - 1 do
            if band(c8.plane, 1) ~= 0 then
                c8.display[i][j] = 0
            end
            if band(c8.plane, 2) ~= 0 then
                c8.display2[i][j] = 0
            end
        end
    end
    c8.draw_flag = true
end

function render()
    local f = string.format
    cls(0)
    local width = c8.extended_mode and 128 or 64
    local height = c8.extended_mode and 64 or 32
    for x = 0, width - 1 do
        for y = 0, height - 1 do
            local pixel = 0
            -- XO-CHIP: Combine two bitplanes
            -- Plane 0 is shown in one color, Plane 1 in another, overlap in third
            if c8.display[x][y] == 1 and c8.display2[x][y] == 1 then
                pixel = 3  -- Both planes (could be different color)
            elseif c8.display[x][y] == 1 then
                pixel = 1
            elseif c8.display2[x][y] == 1 then
                pixel = 2
            else
                pixel = 0
            end
            
            if pixel > 0 then
                local color = 255
                if pixel == 2 then
                    color = 128  -- Plane 1 in different color
                elseif pixel == 3 then
                    color = 64   -- Overlap in yet different color
                end
                
                local SCALE = 3
                if c8.extended_mode then
                    SCALE = 3
                else
                    SCALE = 6
                end
                rect(x*SCALE, y*SCALE, SCALE, SCALE, color)
            end
        end
    end
    
    -- Display mode indicator
    local mode_str = c8.extended_mode and "EXT" or "STD"
    local plane_str = ""
    if c8.plane == 1 then plane_str = "P0" elseif c8.plane == 2 then plane_str = "P1" else plane_str = "P01" end
    
    print("PC="..f("%03X", c8.pc)..",I="..f("%03X", c8.I)..",SP="..f("%X", c8.sp)
    ..",["..mode_str.."]["..plane_str.."] Pitch="..f("%02X", c8.pitch).."("..math.floor(4000 * (2 ^ ((c8.pitch - 64) / 48)) ).."Hz)Vx=", 0, 192, rgb(255,255,255))
    txt = ""
    for i = 0, 15 do
        txt = txt .. f("%02X", c8.V[i]) .. " "
    end
    print(txt, 0, 192+12, rgb(255,255,255))
    txt = "SndBuf="
    for i = 0, 15 do
        txt = txt .. f("%02X", c8.audio_buffer[i]) .. ""
    end
    print(txt, 0, 192+24, rgb(255,255,255))
    print("DT="..c8.delay_timer..",ST="..c8.sound_timer..",IPS:"..c8.step*60 .."("..c8.step.."/f)", 0, 192+36, rgb(255,255,255))
end

-- callback function for each frame
function LOOP()
    if c8.wait_for_key ~= nil then
        return
    end

    for _ = 1, c8.step do
        -- Handle pending long I load
        if c8.pending_long_i ~= nil then
            local byte1 = c8.memory[c8.pc]
            local byte2 = c8.memory[c8.pc + 1]
            c8.I = bor(lshift(byte1, 8), byte2)
            c8.pc = band(c8.pc + 2, 0xFFFF)  -- XO-CHIP: I can now be 16-bit
            c8.pending_long_i = nil
        else
            local op = bor(lshift(c8.memory[c8.pc], 8), c8.memory[c8.pc + 1])
            c8.pc = band(c8.pc + 2, 0xFFFF)  -- XO-CHIP: pc can be 16-bit

            local nnn = band(op, 0x0FFF)
            local nn = band(op, 0x00FF)
            local n = band(op, 0x000F)
            local x = band(rshift(op, 8), 0x000F)
            local y = band(rshift(op, 4), 0x000F)
            local kk = band(op, 0x00FF)

            local high = band(op, 0xF000)
            
            -- 00DN: Scroll display up N pixels (XO-CHIP)
            if op >= 0x00D0 and op <= 0x00DF then
                local n_scroll = band(op, 0x000F)
                scroll_up(n_scroll)
            -- 00CN: Scroll display N lines down (Super-CHIP)
            elseif op >= 0x00C0 and op <= 0x00CF then
                local n_scroll = band(op, 0x000F)
                scroll_down(n_scroll)
            elseif op == 0x00E0 then
                -- 00E0: Clear display
                local width = c8.extended_mode and 128 or 64
                local height = c8.extended_mode and 64 or 32
                for i = 0, width - 1 do
                    for j = 0, height - 1 do
                        if band(c8.plane, 1) ~= 0 then
                            c8.display[i][j] = 0
                        end
                        if band(c8.plane, 2) ~= 0 then
                            c8.display2[i][j] = 0
                        end
                    end
                end
                c8.draw_flag = true
            elseif op == 0x00EE then
                -- 00EE: Return from subroutine
                c8.sp = c8.sp - 1
                c8.pc = c8.stack[c8.sp] or 0x200
            elseif op == 0x00FB then
                -- 00FB: Scroll display 4 pixels right (Super-CHIP)
                scroll_right()
            elseif op == 0x00FC then
                -- 00FC: Scroll display 4 pixels left (Super-CHIP)
                scroll_left()
            elseif op == 0x00FD then
                -- 00FD: Exit CHIP interpreter (Super-CHIP)
                -- For now, we just stop execution
                return
            elseif op == 0x00FE then
                -- 00FE: Disable extended screen mode (Super-CHIP)
                c8.extended_mode = false
                c8.draw_flag = true
            elseif op == 0x00FF then
                -- 00FF: Enable extended screen mode (Super-CHIP)
                c8.extended_mode = true
                c8.draw_flag = true
            elseif high == 0x1000 then
                -- 1NNN: Jump to NNN
                c8.pc = nnn
            elseif high == 0x2000 then
                -- 2NNN: Call subroutine at NNN
                c8.stack[c8.sp] = c8.pc
                c8.sp = c8.sp + 1
                c8.pc = nnn
            elseif high == 0x3000 then
                -- 3XKK: Skip next instruction if VX == KK
                if c8.V[x] == kk then c8.pc = band(c8.pc + 2, 0xFFFF) end
            elseif high == 0x4000 then
                -- 4XKK: Skip next instruction if VX != KK
                if c8.V[x] ~= kk then c8.pc = band(c8.pc + 2, 0xFFFF) end
            elseif high == 0x5000 then
                if n == 0 then
                    -- 5XY0: Skip next instruction if VX == VY
                    if c8.V[x] == c8.V[y] then c8.pc = band(c8.pc + 2, 0xFFFF) end
                elseif n == 2 then
                    -- 5XY2: save vx - vy (XO-CHIP)
                    local start_reg = math.min(x, y)
                    local end_reg = math.max(x, y)
                    for i = start_reg, end_reg do
                        c8.memory[c8.I + (i - start_reg)] = c8.V[i]
                    end
                elseif n == 3 then
                    -- 5XY3: load vx - vy (XO-CHIP)
                    local start_reg = math.min(x, y)
                    local end_reg = math.max(x, y)
                    for i = start_reg, end_reg do
                        c8.V[i] = c8.memory[c8.I + (i - start_reg)]
                    end
                end
            elseif high == 0x6000 then
                -- 6XKK: VX := KK
                c8.V[x] = kk
            elseif high == 0x7000 then
                -- 7XKK: VX := VX + KK
                c8.V[x] = band(c8.V[x] + kk, 0xFF)
            elseif high == 0x8000 then
                -- 8XY_: Arithmetic operations
                if n == 0x0 then
                    -- 8XY0: VX := VY
                    c8.V[x] = c8.V[y]
                elseif n == 0x1 then
                    -- 8XY1: VX := VX or VY
                    c8.V[x] = band(bor(c8.V[x], c8.V[y]), 0xFF)
                elseif n == 0x2 then
                    -- 8XY2: VX := VX and VY
                    c8.V[x] = band(band(c8.V[x], c8.V[y]), 0xFF)
                elseif n == 0x3 then
                    -- 8XY3: VX := VX xor VY
                    c8.V[x] = band(bxor(c8.V[x], c8.V[y]), 0xFF)
                elseif n == 0x4 then
                    -- 8XY4: VX := VX + VY, VF := carry
                    local sum = c8.V[x] + c8.V[y]
                    c8.V[0xF] = (sum > 0xFF) and 1 or 0
                    c8.V[x] = band(sum, 0xFF)
                elseif n == 0x5 then
                    -- 8XY5: VX := VX - VY, VF := not borrow
                    c8.V[0xF] = (c8.V[x] >= c8.V[y]) and 1 or 0
                    c8.V[x] = band(c8.V[x] - c8.V[y], 0xFF)
                elseif n == 0x6 then
                    -- 8XY6: VX := VX shr 1, VF := carry
                    c8.V[0xF] = band(c8.V[x], 0x1)
                    c8.V[x] = band(rshift(c8.V[x], 1), 0xFF)
                elseif n == 0x7 then
                    -- 8XY7: VX := VY - VX, VF := not borrow
                    c8.V[0xF] = (c8.V[y] >= c8.V[x]) and 1 or 0
                    c8.V[x] = band(c8.V[y] - c8.V[x], 0xFF)
                elseif n == 0xE then
                    -- 8XYE: VX := VX shl 1, VF := carry
                    c8.V[0xF] = band(rshift(c8.V[x], 7), 0x1)
                    c8.V[x] = band(lshift(c8.V[x], 1), 0xFF)
                end
            elseif high == 0x9000 then
                -- 9XY0: Skip next instruction if VX != VY
                if n == 0 and c8.V[x] ~= c8.V[y] then c8.pc = band(c8.pc + 2, 0xFFFF) end
            elseif high == 0xA000 then
                -- ANNN: I := NNN
                c8.I = nnn
            elseif high == 0xB000 then
                -- BNNN: Jump to NNN+V0
                c8.pc = band(nnn + c8.V[0], 0xFFFF)
            elseif high == 0xC000 then
                -- CXKK: VX := pseudorandom_number and KK
                local r = math.random(0, 255)
                c8.V[x] = band(band(r, kk), 0xFF)
            elseif high == 0xD000 then
            -- DXYN: Show N-byte sprite (or 16x16 if N=0 in extended mode)
            local vx = c8.V[x]
            local vy = c8.V[y]
            local width = c8.extended_mode and 128 or 64
            local height = c8.extended_mode and 64 or 32
            
            c8.V[0xF] = 0
            
            if n == 0 and c8.extended_mode then
                -- 16x16 sprite in extended mode
                local sprite_data_offset = 0
                -- Draw to plane 0
                if band(c8.plane, 1) ~= 0 then
                    for row = 0, 15 do
                        local sprite = c8.memory[c8.I + row * 2] or 0
                        for col = 0, 7 do
                            local bit = band(rshift(sprite, 7 - col), 0x1)
                            if bit == 1 then
                                local px = (vx + col) % width
                                local py = (vy + row) % height
                                if c8.display[px][py] == 1 then
                                    c8.V[0xF] = 1
                                end
                                c8.display[px][py] = bxor(c8.display[px][py], 1)
                            end
                        end
                        local sprite2 = c8.memory[c8.I + row * 2 + 1] or 0
                        for col = 8, 15 do
                            local bit = band(rshift(sprite2, 23 - col), 0x1)
                            if bit == 1 then
                                local px = (vx + col) % width
                                local py = (vy + row) % height
                                if c8.display[px][py] == 1 then
                                    c8.V[0xF] = 1
                                end
                                c8.display[px][py] = bxor(c8.display[px][py], 1)
                            end
                        end
                    end
                end
                -- Draw to plane 1 if selected
                if band(c8.plane, 2) ~= 0 then
                    for row = 0, 15 do
                        local sprite = c8.memory[c8.I + 32 + row * 2] or 0
                        for col = 0, 7 do
                            local bit = band(rshift(sprite, 7 - col), 0x1)
                            if bit == 1 then
                                local px = (vx + col) % width
                                local py = (vy + row) % height
                                if c8.display2[px][py] == 1 then
                                    c8.V[0xF] = 1
                                end
                                c8.display2[px][py] = bxor(c8.display2[px][py], 1)
                            end
                        end
                        local sprite2 = c8.memory[c8.I + 32 + row * 2 + 1] or 0
                        for col = 8, 15 do
                            local bit = band(rshift(sprite2, 23 - col), 0x1)
                            if bit == 1 then
                                local px = (vx + col) % width
                                local py = (vy + row) % height
                                if c8.display2[px][py] == 1 then
                                    c8.V[0xF] = 1
                                end
                                c8.display2[px][py] = bxor(c8.display2[px][py], 1)
                            end
                        end
                    end
                end
            else
                -- 8xN sprite (standard)
                if band(c8.plane, 1) ~= 0 then
                    for row = 0, n - 1 do
                        local sprite = c8.memory[c8.I + row] or 0
                        for col = 0, 7 do
                            local bit = band(rshift(sprite, 7 - col), 0x1)
                            if bit == 1 then
                                local px = (vx + col) % width
                                local py = (vy + row) % height
                                if c8.display[px][py] == 1 then
                                    c8.V[0xF] = 1
                                end
                                c8.display[px][py] = bxor(c8.display[px][py], 1)
                            end
                        end
                    end
                end
                if band(c8.plane, 2) ~= 0 then
                    for row = 0, n - 1 do
                        local sprite = c8.memory[c8.I + row + 16] or 0
                        for col = 0, 7 do
                            local bit = band(rshift(sprite, 7 - col), 0x1)
                            if bit == 1 then
                                local px = (vx + col) % width
                                local py = (vy + row) % height
                                if c8.display2[px][py] == 1 then
                                    c8.V[0xF] = 1
                                end
                                c8.display2[px][py] = bxor(c8.display2[px][py], 1)
                            end
                        end
                    end
                end
            end
            c8.draw_flag = true
        elseif high == 0xE000 then
            -- EX9E / EXA1: Key operations
            if kk == 0x9E then
                -- EX9E: Skip next instruction if key VX pressed
                if c8.keys[c8.V[x]] == 1 then c8.pc = band(c8.pc + 2, 0xFFFF) end
            elseif kk == 0xA1 then
                -- EXA1: Skip next instruction if key VX not pressed
                if c8.keys[c8.V[x]] == 0 then c8.pc = band(c8.pc + 2, 0xFFFF) end
            end
        elseif op == 0xF000 then
            -- F000 NNNN: Load I with 16-bit value (XO-CHIP)
            c8.pending_long_i = true
        elseif high == 0xF000 then
            -- FX__: Various operations
            if kk == 0x01 then
                -- FN01: plane n (XO-CHIP) - select drawing planes
                c8.plane = band(n, 0x3)  -- 0-3, where 0=none, 1=plane0, 2=plane1, 3=both
            elseif kk == 0x02 then
                -- FX02: audio (XO-CHIP) - load 16 bytes from I to audio pattern buffer
                for i = 0, 15 do
                    c8.audio_buffer[i] = c8.memory[c8.I + i] or 0
                end
                setup_pcm_audio()
            elseif kk == 0x07 then
                -- FX07: VX := delay_timer
                c8.V[x] = c8.delay_timer
            elseif kk == 0x0A then
                -- FX0A: wait for keypress, store hex value of key in VX
                c8.wait_for_key = x
                return
            elseif kk == 0x15 then
                -- FX15: delay_timer := VX
                c8.delay_timer = c8.V[x]
            elseif kk == 0x18 then
                -- FX18: sound_timer := VX
                c8.sound_timer = c8.V[x]
            elseif kk == 0x1E then
                -- FX1E: I := I + VX
                c8.I = c8.I + c8.V[x]
            elseif kk == 0x29 then
                -- FX29: Point I to 5-byte font sprite for hex character VX
                c8.I = band(c8.V[x] * 5, 0xFFF)
            elseif kk == 0x30 then
                -- FX30: Point I to 10-byte font sprite for digit VX (0..9) [Super-CHIP]
                if c8.V[x] <= 9 then
                    c8.I = band(0x50 + c8.V[x] * 10, 0xFFF)
                end
            elseif kk == 0x33 then
                -- FX33: Store BCD representation of VX in M(I)..M(I+2)
                local v = c8.V[x]
                c8.memory[c8.I] = math.floor(v / 100)
                c8.memory[c8.I + 1] = math.floor((v % 100) / 10)
                c8.memory[c8.I + 2] = v % 10
            elseif kk == 0x3A then
                -- FX3A: pitch := vx (XO-CHIP) - set audio pitch
                c8.pitch = c8.V[x]
            elseif kk == 0x55 then
                -- FX55: Store V0..VX in memory starting at M(I)
                for i = 0, x do
                    c8.memory[c8.I + i] = c8.V[i]
                end
            elseif kk == 0x65 then
                -- FX65: Read V0..VX from memory starting at M(I)
                for i = 0, x do
                    c8.V[i] = c8.memory[c8.I + i] or 0
                end
            elseif kk == 0x75 then
                -- FX75: saveflags vx - Store V0..VX in RPL user flags [generalized for XO-CHIP]
                for i = 0, math.min(x, 15) do
                    c8.flag_registers[i] = c8.V[i]
                end
            elseif kk == 0x85 then
                -- FX85: loadflags vx - Read V0..VX from RPL user flags [generalized for XO-CHIP]
                for i = 0, math.min(x, 15) do
                    c8.V[i] = c8.flag_registers[i]
                end
            elseif kk == 0x9E then
                -- FX9E: Skip next instruction if key VX pressed
                if c8.keys[c8.V[x]] == 1 then c8.pc = band(c8.pc + 2, 0xFFFF) end
            elseif kk == 0xA1 then
                -- FXA1: Skip next instruction if key VX not pressed
                if c8.keys[c8.V[x]] == 0 then c8.pc = band(c8.pc + 2, 0xFFFF) end
            end
        end
        end  -- End of pending_long_i else block
    end  -- End of step loop
    render()
    if c8.delay_timer > 0 then c8.delay_timer = c8.delay_timer - 1 end
    if c8.sound_timer > 0 then 
        c8.sound_timer = c8.sound_timer - 1 
        beep() -- XO-CHIP: PCM pattern playback
    else
        poke(0x400202, 0)  -- Stop CH9
    end
end

function ONKEYDOWN(k)
    local name = to_key_name(k)
    local map = {
        ["1"] = 0x1, ["2"] = 0x2, ["3"] = 0x3, ["4"] = 0xC,
        ["Q"] = 0x4, ["W"] = 0x5, ["E"] = 0x6, ["R"] = 0xD,
        ["A"] = 0x7, ["S"] = 0x8, ["D"] = 0x9, ["F"] = 0xE,
        ["Z"] = 0xA, ["X"] = 0x0, ["C"] = 0xB, ["V"] = 0xF
    }
    local key = map[name]
    if key ~= nil then
        c8.keys[key] = 1
        if c8.wait_for_key ~= nil then
            c8.V[c8.wait_for_key] = key
            c8.wait_for_key = nil
        end
    end
end

function ONKEYUP(k)
    local name = to_key_name(k)
    local map = {
        ["1"] = 0x1, ["2"] = 0x2, ["3"] = 0x3, ["4"] = 0xC,
        ["Q"] = 0x4, ["W"] = 0x5, ["E"] = 0x6, ["R"] = 0xD,
        ["A"] = 0x7, ["S"] = 0x8, ["D"] = 0x9, ["F"] = 0xE,
        ["Z"] = 0xA, ["X"] = 0x0, ["C"] = 0xB, ["V"] = 0xF
    }
    local key = map[name]
    if key ~= nil then
        c8.keys[key] = 0
    end
    if name == "Up" then
        c8.step = math.min(c8.step + 1, 10000)
    elseif name == "Down" then
        c8.step = math.max(c8.step - 1, 1)
    elseif name == "Right" then
        c8.step = math.min(c8.step + 10, 10000)
    elseif name == "Left" then
        c8.step = math.max(c8.step - 10, 1)
    elseif name == "PageUp" then
        c8.step = math.min(c8.step + 100, 10000)
    elseif name == "PageDown" then
        c8.step = math.max(c8.step - 100, 1)
    end
end