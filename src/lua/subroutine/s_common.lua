function S_COMMON_clip(v,a,b)
    return math.min(math.max(v,a),b)
end

function S_COMMON_tableMax(tbl)
    local max = nil
    for k, v in pairs(tbl) do
        if max == nil or v > max then
            max = v
        end
    end
    return max
end

function S_COMMON_tableMin(tbl)
    local min = nil
    for k, v in pairs(tbl) do
        if min == nil or v < min then
            min = v
        end
    end
    return min
end

function S_COMMON_tableAvg(tbl)
    local sum = 0
    local count = 0
    for k, v in pairs(tbl) do
        sum = sum + v
        count = count + 1
    end
    if count == 0 then
        return 0
    end
    return sum / count
end

function S_COMMON_avg_abs(sample)
    local sum = 0
    for i=1,#sample do
        sum = sum + math.abs(sample[i])
    end
    return sum / #sample
end