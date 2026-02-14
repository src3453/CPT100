screen(2)

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
    elseif to_key_name(k) == "Return" then
        printlnp("")
    end
end