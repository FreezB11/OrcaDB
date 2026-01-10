math.randomseed(os.time())

request = function()
  local k = math.random(1, 1000000)
  return wrk.format(
    "GET",
    "/api/v1/GET?key=wrk" .. k
  )
end

-- wrk -t4 -c64 -d30s -s get.lua http://localhost:8080