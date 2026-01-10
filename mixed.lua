math.randomseed(os.time())

put_counter = 0
get_counter = 0

request = function()
  -- 10% PUT, 90% GET
  if math.random(1,10) == 1 then
    put_counter = put_counter + 1
    return wrk.format(
      "PUT",
      "/api/v1/PUT?key=mix" .. put_counter .. "&value=" .. put_counter
    )
  else
    if put_counter == 0 then
      return wrk.format("GET", "/api/v1/GET?key=mix1")
    end
    local k = math.random(1, put_counter)
    return wrk.format(
      "GET",
      "/api/v1/GET?key=mix" .. k
    )
  end
end

-- wrk -t4 -c64 -d30s -s mixed.lua http://localhost:8080