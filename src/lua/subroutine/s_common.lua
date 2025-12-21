
function S_COMMON_clip(v,a,b)
    return math.min(math.max(v,a),b)
end

function S_COMMON_tableMax(tbl)
    local max = nil
    for _, v in pairs(tbl) do
        if max == nil or v > max then
            max = v
        end
    end
    return max
end

function S_COMMON_tableMin(tbl)
    local min = nil
    for _, v in pairs(tbl) do
        if min == nil or v < min then
            min = v
        end
    end
    return min
end

function S_COMMON_tableAvg(tbl)
    local sum = 0
    local count = 0
    for _, v in pairs(tbl) do
        sum = sum + v
        count = count + 1
    end
    if count == 0 then
        return 0
    end
    return sum / count
end

function S_COMMON_avg_abs(sample)
    local sum = 0
    for i=1,#sample do
        sum = sum + math.abs(sample[i])
    end
    return sum / #sample
end

function S_COMMON_peek4(addr)
-- Peek a 4bit nibble from memory, addr is 2x aligned
    local byte = peek(addr/2)
    if addr % 2 == 0 then
        return bit.band(byte / 16, 0x0F)
    else
        return bit.band(byte, 0x0F)
    end
end

function S_COMMON_poke4(addr, val)
-- Poke a 4bit nibble into memory, addr is 2x aligned
    local byte = peek(addr/2)
    if addr % 2 == 0 then
        byte = bit.bor(bit.band(byte, 0x0F), bit.lshift(bit.band(val, 0x0F), 4))
    else
        byte = bit.bor(bit.band(byte, 0xF0), bit.band(val, 0x0F))
    end
    poke(addr/2, byte)
end

function S_COMMON_vpeek4(addr)
-- Peek a 4bit nibble from video memory, addr is 2x aligned
    local byte = vpeek(addr/2)
    if addr % 2 == 0 then
        return bit.band(byte / 16, 0x0F)
    else
        return bit.band(byte, 0x0F)
    end
end

function S_COMMON_vpoke4(addr, val)
-- Poke a 4bit nibble into video memory, addr is 2x aligned
    local byte = vpeek(addr/2)
    if addr % 2 == 0 then
        byte = bit.bor(bit.band(byte, 0x0F), bit.lshift(bit.band(val, 0x0F), 4))
    else
        byte = bit.bor(bit.band(byte, 0xF0), bit.band(val, 0x0F))
    end
    vpoke(addr/2, byte)
end