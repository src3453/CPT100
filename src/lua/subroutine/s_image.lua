function S_IMAGE_HSV2RGB(h, s, v)
    local r, g, b

    h = S_COMMON_clip(h,0,1)
    s = S_COMMON_clip(s,0,1)
    v = S_COMMON_clip(v,0,1)

    local i = math.floor(h * 6)
    local f = h * 6 - i
    local p = v * (1 - s)
    local q = v * (1 - f * s)
    local t = v * (1 - (1 - f) * s)

    i = i % 6

    if i == 0 then r, g, b = v, t, p
    elseif i == 1 then r, g, b = q, v, p
    elseif i == 2 then r, g, b = p, v, t
    elseif i == 3 then r, g, b = p, q, v
    elseif i == 4 then r, g, b = t, p, v
    elseif i == 5 then r, g, b = v, p, q
    end

    return math.floor(r * 255), math.floor(g * 255), math.floor(b * 255)
end

function S_IMAGE_RGB2HSV(r, g, b)
    r = S_COMMON_clip(r/255,0,1)
    g = S_COMMON_clip(g/255,0,1)
    b = S_COMMON_clip(b/255,0,1)

    local max = math.max(r, g, b)
    local min = math.min(r, g, b)
    local h, s, v = 0, 0, max

    local d = max - min
    if max ~= 0 then
        s = d / max
    else
        s = 0
        h = -1
        return h, s, v
    end

    if r == max then
        h = (g - b) / d
    elseif g == max then
        h = 2 + (b - r) / d
    else
        h = 4 + (r - g) / d
    end

    h = h / 6
    if h < 0 then
        h = h + 1
    end

    return h, s, v
end

function S_IMAGE_PrintShadow(txt, x, y, color, color2)
    color = color or rgb(255,255,255)
    color2 = color2 or rgb(0,0,0)
    print(txt,x+1,y+1,color2)
    print(txt,x,y,color)
end