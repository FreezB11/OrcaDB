counter = 0

request = function()
  counter = counter + 1
  local key = "wrk" .. counter
  local value = counter
  return wrk.format(
    "PUT",
    "/api/v1/PUT?key=" .. key .. "&value=" .. value
  )
end

-- wrk -t4 -c64 -d30s -s put.lua http://localhost:8080