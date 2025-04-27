
t=0
resolution = 3
dither = {
    {0,8,2,10},
    {12,4,14,6},
    {3,11,1,9},
    {15,7,13,5}
}

-- This is a simple Lua script that initializes a game loop and clears the screen.
function LOOP()
    cls(0)
    xm,ym,_ = mouse()
    for x=0,383,resolution do
        for y=0,287,resolution do
            val = math.max(math.min((math.sin(y/100*ym/50+t/50)+math.cos(x/100*xm/50+t/50))*64+128,255),0)
            fval = 0
            if val%42 >= dither[(y/resolution)%4+1][(x/resolution)%4+1]*(42/16) then
                fval = val//42*42+42
            else
                fval = val//42*42
            end
            fval = math.max(math.min(fval,255),0)
            rect(x,y,resolution,resolution,rgb(0,fval,0))
        end
    end
    print("tick:"..t,1,1,rgb(0,0,0))
    print("tick:"..t,0,0,rgb(255,255,255))
    t=t+1
end