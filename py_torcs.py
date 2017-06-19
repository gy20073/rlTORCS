import lua
import gym
from gym import spaces
import numpy as np

class TorcsEnv(gym.Env):
    '''
    sample config dict
    lg.opt = {'server': server,
              'game_config': game_config,
              'mkey': mkey,
              'auto_back': auto_back}
    # some constants:
        use_RGB=True
    '''
    

    def __init__(self, subtype="discrete", **kwargs):
        self.sample_luaTable_type = type(lua.toTable({}))
    
        self.lg = lua.globals()
        kwargs.update(use_RGB=True)
        self.lg.opt = lua.toTable(kwargs)
        self.subtype = subtype
        self.viewer = None

        if subtype == "discrete":
            self.lg.env_class = lua.require("TORCS.TorcsDiscrete")
            lua.execute(" env = env_class(opt) ")
            self.action_space = spaces.Discrete(9)
        elif subtype == "continuous":
            raise NotImplemented("continuous subtype action has not been implemented")
        else:
            raise ValueError("invalid subtype of the environment, subtype = ", subtype)

        self.observation_space = spaces.Box(low=0, high=255, shape=(480, 640, 3))

    def _reset(self):
        obs = lua.eval("env:start()")
        return self._convert_obs(obs)

    def _step(self, u):
        assert self.action_space.contains(u)
        if self.subtype == "discrete":
            # convert 0 based indexing to 1 based indexing in rlTorcs
            u += 1

        self.lg.u = u
        lua.execute("res1, res2, res3 = env:step(u)")
        reward, observation, terminal = self.lg.res1, self.lg.res2, self.lg.res3

        return self._convert_obs(observation), reward, terminal, {}

    def _render(self, mode="human", close=False):
        if close:
            if self.viewer is not None:
                self.viewer.close()
                self.viewer = None
            return
        img = self._get_image()
        if mode == 'rgb_array':
            return img
        elif mode == 'human':
            from gym.envs.classic_control import rendering
            if self.viewer is None:
                self.viewer = rendering.SimpleImageViewer()
            self.viewer.imshow(img)

    #def _seed(self):

    def _close(self):
        lua.execute(" env:cleanUp() ")

    def __del__(self):
        self.close()

    ######################## util functions ########################
    def _convert_obs(self, obs):
        obs = np.array(obs)
        # From 3*480*640 to 480*640*3
        obs = np.transpose(obs, [1, 2, 0])
        obs *= 255.0
        obs = obs.astype(np.uint8)
        return obs

    def _get_image(self):
        observation_RGB = np.zeros((3, 480, 640), dtype=np.float32)
        self.call_ctrl("getRGBImage", observation_RGB, 0)
        return self._convert_obs(observation_RGB)

    def _convert_var(self, var):
        if isinstance(var, dict):
            #print("converting dict to table")
            var = lua.toTable(var)

        if isinstance(var, self.sample_luaTable_type):
            #print("converting table to dict")
            var = lua.toDict(var)
        return var

    def call_ctrl(self, func, *args):
        # general mapping from python command to lua control model
        cmd = "env.ctrl."+func+"("
        n = len(args)

        for i in range(n):
            # push all args into the environment
            name = "arg"+str(i)
            var = self._convert_var(args[i])
            setattr(self.lg, name, var)
            # and construct the calling command
            cmd += name + ","
            
        cmd = cmd[:-1] + ")"
        #print("calling command:", cmd)

        # multiple return results, not an issue here
        output = lua.eval(cmd)
        return self._convert_var(output)