-- test.lua
local TorcsDiscrete = require 'TORCS.TorcsDiscrete'

-- configure the environment
local opt = {}
opt.server = true
opt.game_config = 'quickrace_discrete_single_ushite-city.xml'
opt.use_RGB = false
opt.mkey = 817
opt.auto_back = false

local env = TorcsDiscrete(opt)
local totalstep = 817
print("begin")
local reward, terminal, state = 0, false, env:start()
repeat
	repeat
		reward, observation, terminal = env:step(doSomeActions(state))
		nowstep = nowstep + 1
	until terminal

	if terminal then
		reward, terminal, state = 0, false, env:start()
	end
until nowstep >= totalstep
print("finish")
env:cleanUp()

