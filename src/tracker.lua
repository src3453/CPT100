mode=0
cur0=0

function LOOP()

end

function ONKEYDOWN(k)
    if to_key_name(k) == "Up" then
        if mode == 0 then
            cur0=(cur0-6)%1536
        end
    end
    if to_key_name(k) == "Down" then
        if mode == 0 then
            cur0=(cur0+6)%1536
        end
    end
    if to_key_name(k) == "Left" then
        if mode == 0 then
            cur0=(cur0-1)%1536
        end
    end
    if to_key_name(k) == "Right" then
        if mode == 0 then
            cur0=(cur0+1)%1536
        end
    end
end