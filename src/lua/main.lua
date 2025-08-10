-- CPT200 Demo: Flappy Bird

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

function spawn_obstacle()
    -- Create a new obstacle at a random height
    -- in Flappy Bird, the obstacles are gaps between pipes
    local gap_y = math.random(64, 224)
    -- Create the top and bottom parts of the obstacle
    local top = {
        x = 384,
        y = 0,
        w = 16,
        h = gap_y - 32
    }
    local bottom = {
        x = 384,
        y = gap_y,
        w = 16,
        h = 288 - gap_y
    }
    table.insert(obstacles, top)
    table.insert(obstacles, bottom)
end

function update_obstacles()
    for i, obs in ipairs(obstacles) do
        obs.x = obs.x - 2 -- Move obstacles left
    end
    if obstacles[1] and obstacles[1].x < -16 then
        table.remove(obstacles, 1) -- Remove off-screen obstacles
    end
end

function draw_obstacles()
    for i, obs in ipairs(obstacles) do
        rect(obs.x, obs.y, obs.w, obs.h, rgb(0,255,0))
    end
end

function obstacle_collides_with_player()
    for i, obs in ipairs(obstacles) do
        -- Check if the obstacle is colliding with the player, with two AABB objects
        if AABBcollision(obs, { x = player_x, y = player_y, w = 8, h = 8 }) then
            return true
        end
    end
    -- If no collision was detected, return false
    return false
end

function update_and_draw_player()
    if player_vy > -2 then
        player_vy = player_vy - 0.05
    end
    player_y = player_y - player_vy
    player_y = player_y % 288
    rect(player_x, player_y, 8, 8, rgb(255,255,0)) -- Draw player

end

obstacles = {}

player_x = 0
player_y = 0
player_vy = 0
score = 0
previous_score = 0
timer_gameover = 0
t = 0

function BOOT()
    screen(0)
    cls(rgb(0,128,255))
    player_x = 16

end

function LOOP()
    cls(rgb(0,128,255)) -- Clear screen

    
    if timer_gameover > 0 then
        timer_gameover = timer_gameover - 1
        print("Game Over! Score: " .. int(previous_score), 100, 138, rgb(255,0,0))
    else
        if t % 60 == 0 then
            spawn_obstacle() -- Spawn a new obstacle every second
        end
        update_and_draw_player()
        update_obstacles()
        draw_obstacles()
        score=score+1
    end
    if obstacle_collides_with_player() then
        -- Handle player collision with obstacle
        if timer_gameover == 0 then
            obstacles = {} -- Reset obstacles
            player_y = 144
            player_vy = 0
            previous_score = score
            score = 0 -- Reset score
            timer_gameover = 60 -- Set game over timer for 1 second
        end
    end
    print("Flappy Bird: CPT200 Tech Demo", 0, 0, rgb(255,255,255))
    print("Score: " .. int(score), 0, 12, rgb(255,255,255))
    
    t = t + 1
end

function ONKEYDOWN(k)
    if to_key_name(k) == "X" then
        player_vy = 2
    end
end