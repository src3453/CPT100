AABB = {
    -- Axis-Aligned Bounding Box struct
    x = 0,
    y = 0,
    w = 0,
    h = 0
}

function AABBcollision(a, b)
    -- Check if two AABBs are colliding
    return a.x < b.x + b.w and
           a.x + a.w > b.x and
           a.y < b.y + b.h and
           a.y + a.h > b.y
end