
t=0
resolution = 3
dither = {
    {0,8,2,10},
    {12,4,14,6},
    {3,11,1,9},
    {15,7,13,5}
}

for i=0,255 do
    r = math.min(i*1, 255)
    g = math.min(i*2, 255)
    b = math.min(i*3, 255)
    vpoke(0x1b000+i*3+0,r)
    vpoke(0x1b000+i*3+1,g)
    vpoke(0x1b000+i*3+2,b)
end
-- This is a simple Lua script that initializes a game loop and clears the screen.
function LOOP()
    cls(0)
    xm,ym,_ = mouse()
    for x=0,383,resolution do
        for y=0,287,resolution do
            val = math.max(math.min((math.sin(y/100*ym/50+t/50)+math.cos(x/100*xm/50+t/50))*64+128,255),0)
            fval = 0
            if val%1 >= dither[(y/resolution)%4+1][(x/resolution)%4+1]*(1/16) then
                fval = val//1*1+1
            else
                fval = val//1*1
            end
            fval = math.max(math.min(fval,255),0)
            rect(x,y,resolution,resolution,fval)
        end
    end
    print("tick:"..t,1,1,rgb(0,0,0))
    print("tick:"..t,0,0,rgb(255,255,255))
    t=t+1
end