lc(0,0)
cls(0)
printlnp("This is a test of text mode")
colorfg(rgb(0,255,0))
printlnp("This is a another test of text mode")
lc(0,1)
startinput()
colorfg(255)

function ONINPUT(c)
    printp(c)
end

function BOOT()
    
end

function LOOP()
    
end

function ONKEYDOWN(k)
    if to_key_name(k) == "Up" then
        movecursor(0, -1)
    elseif to_key_name(k) == "Down" then
        movecursor(0, 1)
    elseif to_key_name(k) == "Left" then
        movecursor(-1, 0)
    elseif to_key_name(k) == "Right" then
        movecursor(1, 0)
    elseif to_key_name(k) == "Enter" then
        printlnp("")
    end
end