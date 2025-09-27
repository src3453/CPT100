function BOOT()
    screen(1) -- change to text mode
    cls(0) -- clear screen
    showcurp(0) -- hide cursor
end

dither = {
    {0,8,2,10},
    {12,4,14,6},
    {3,11,1,9},
    {15,7,13,5}
}

t = 0
t2 = 0

chars = {0x20, 0xb0, 0xb1, 0xb2, 0xdb}

function LOOP()
    t = 0
    for y=0, 23 do
        for x=0, 47 do
            val = math.sin((x+t2)/3.14/1)+math.sin((y+t2)/3.14/1)+2
            fval = 0
            if val%1 >= dither[(y)%4+1][(x)%4+1]*(1/16) then
                fval = val//1*1+1
            else
                fval = val//1*1
            end
            vpoke(0x1c000+(y*48+x)*3, chars[fval%5+1])
            --vpoke(0x1c000+(y*48+x)*3+1, t)
            t = t + 1
        end
    end
    t2 = t2 + 0.05
end