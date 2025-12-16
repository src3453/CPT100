function S_GAME_AABBcollision(a, b)
    -- Check if two AABBs are colliding
    return a.x < b.x + b.w and
           a.x + a.w > b.x and
           a.y < b.y + b.h and
           a.y + a.h > b.y
end