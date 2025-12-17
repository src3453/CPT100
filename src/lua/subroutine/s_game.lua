function S_GAME_AABBcollision(a, b)
    -- Check if two AABBs are colliding
    return a.x < b.x + b.w and
           a.x + a.w > b.x and
           a.y < b.y + b.h and
           a.y + a.h > b.y
end

function S_GAME_animationEaseInOut(t, b, c, d)
    -- Easing function for smooth animations (ease in-out quad), t=current time, b=start value, c=change in value, d=duration
    t = t / (d / 2)
    if t < 1 then
        return c / 2 * t * t + b
    end
    t = t - 1
    return -c / 2 * (t * (t - 2) - 1) + b
end