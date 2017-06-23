-- luacheck: ignore __threadid log action_description super
-- luacheck: globals Env
local classic = require 'classic'
local TorcsDiscrete = require 'TORCS.TorcsDiscrete'
local image = require 'image'
local TorcsDiscreteConstDamagePos, super = classic.class('TorcsDiscreteConstDamagePos', TorcsDiscrete)

-- WARNING: we should never use posix package here!!! (i.e. never call `posix = require 'posix'` etc.)
-- otherwise the signal handler outside the thread will be blocked.

-- Constructor
function TorcsDiscreteConstDamagePos:_init(opts)
	super._init(self, opts)
	-- enable the line below will have strange error
	-- classic.strict(self)
end

function TorcsDiscreteConstDamagePos:reward()
	if (self.ctrl.getDamage() - self.damage > 0 or self:isStuck()) then
        return -0.025
    end
	local speedReward = (self.ctrl.getSpeed() * math.cos(self.ctrl.getAngle()))
	local pos = math.abs(self.ctrl.getPos())
	if speedReward~=speedReward or pos~=pos then
		print("Nan reward, angle: " .. tostring(self.ctrl.getAngle()) .. "speed: " .. tostring(self.ctrl.getSpeed()) .. "pos: " ..  tostring(self.ctrl.getPos()))
	end
	return (speedReward - pos * 0.3) * 0.006
end

return TorcsDiscreteConstDamagePos