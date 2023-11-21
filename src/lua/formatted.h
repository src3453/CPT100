std::string source = "mode=0\n"+
"cur0=0\n"+
"\n"+
"function TrackEditor()\n"+
"    print(\"FRM FM1 FM2 FM3 FM4 WT5 WT6\",16,4,rgb(192,255,192))\n"+
"    rectb(15,19+(cur0//6%16)*16,209,12,250)\n"+
"    rect(47+(cur0%6)*32,19+(cur0//6%16)*16,17,12,250)\n"+
"    for y=0,15 do\n"+
"        print(string.format(\"%02X\",int(math.floor(cur0//96)*16+y)),16,20+y*16,rgb(192,192,255))\n"+
"        for x=0,5 do\n"+
"            print(string.format(\"%02X\",peek(int(0x0F000+math.floor(cur0//96)*96+y*6+x))),48+x*32,20+y*16,255)\n"+
"        end\n"+
"    end\n"+
"    --print(\"mouse:(\"..mx..\",\"..my..\",\"..mb..\")\",mx,my,255)\n"+
"end\n"+
"\n"+
"function PatternEditor()\n"+
"\n"+
"end\n"+
"\n"+
"function drawcur()\n"+
"    mx,my,mb=mouse()\n"+
"    for y=-1,1 do\n"+
"        for x=-1,1 do\n"+
"            line(mx+x,my+y,mx+8+x,my+y,0)\n"+
"            line(mx+x,my+y,mx+x,my+8+y,0)\n"+
"            line(mx+x,my+y,mx+16+x,my+16+y,0)\n"+
"        end\n"+
"    end\n"+
"    line(mx,my,mx+8,my,255)\n"+
"    line(mx,my,mx,my+8,255)\n"+
"    line(mx,my,mx+16,my+16,255)\n"+
"    \n"+
"end\n"+
"\n"+
"showcur(0)\n"+
"startinput()\n"+
"modeLabel = {\n"+
"    \"Track\",\n"+
"    \"Pattern\",\n"+
"}\n"+
"function LOOP()\n"+
"    cls(0)\n"+
"    if mode == 0 then\n"+
"        TrackEditor()\n"+
"    end\n"+
"    if mode == 1 then\n"+
"        PatternEditor()\n"+
"    end\n"+
"    rect(0,275,384,13,255)\n"+
"    print(\"Mode:\"..modeLabel[mode+1],1,276,0)\n"+
"    drawcur()\n"+
"end\n"+
"\n"+
"inputchar = \"\"\n"+
"\n"+
"function ONINPUT(c)\n"+
"    if tonumber(c,16) ~= nil then\n"+
"        inputchar = inputchar .. c\n"+
"        poke(int(0x0F000+cur0),tonumber(inputchar,16))\n"+
"    else\n"+
"\n"+
"    end\n"+
"    if #inputchar == 2 then\n"+
"        inputchar = \"\"\n"+
"    end\n"+
"end\n"+
"\n"+
"function ONKEYDOWN(k)\n"+
"    if to_key_name(k) == \"Up\" then\n"+
"        if mode == 0 then\n"+
"            cur0=(cur0-6)%1536\n"+
"        end\n"+
"    end\n"+
"    if to_key_name(k) == \"Down\" then\n"+
"        if mode == 0 then\n"+
"            cur0=(cur0+6)%1536\n"+
"        end\n"+
"    end\n"+
"    if to_key_name(k) == \"Left\" then\n"+
"        if mode == 0 then\n"+
"            cur0=(cur0-1)%1536\n"+
"        end\n"+
"    end\n"+
"    if to_key_name(k) == \"Right\" then\n"+
"        if mode == 0 then\n"+
"            cur0=(cur0+1)%1536\n"+
"        end\n"+
"    end\n"+
"    if to_key_name(k) == \"Z\" then\n"+
"\n"+
"        mode=(mode+1)%2\n"+
"\n"+
"    end\n"+
"end\n";