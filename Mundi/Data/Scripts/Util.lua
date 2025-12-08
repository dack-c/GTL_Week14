local M = {}

function M.Clamp(value, min, max)
if value <= min then
return min
end
if value >= max then
return max
end
return value
end

function M.Lerp(a, b, t)
    return a + (b - a) * t
end

function M.LerpClamped(a, b, t)
    if t < 0 then t = 0 end
    if t > 1 then t = 1 end
    return a + (b - a) * t
end

function M.Smoothstep(a, b, t)
    t = t * t * (3 - 2 * t)
    return a + (b - a) * t
end


return M