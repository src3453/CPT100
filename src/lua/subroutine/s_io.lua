function S_IO_loadandrun(filepath)
    local file=io.open(filepath,"r")
    if file ~= nil then
        data=file:read("a")
        include(data)
        file:close()
    end
end