#!/usr/bin/lua

local function get_basic_net_info(network, iface, accumulator)
  local net = network:get_network(iface)
  local device = net and net:get_interface()

  if device then
    accumulator["ipaddrs"] = {}

    for _, ipaddr in ipairs(device:ipaddrs()) do
      accumulator.ipaddrs[#accumulator.ipaddrs + 1] = {
        addr = ipaddr:host():string(),
        netmask = ipaddr:mask():string()
      }
    end
  end
end

local function collect_wifi_info()
  local network = require"luci.model.network".init()
  local accumulator = {}
  get_basic_net_info(network, "lan", accumulator)
  return accumulator
end

local info = collect_wifi_info()

if #info.ipaddrs > 0 then
  print(info.ipaddrs[1].addr)
end
