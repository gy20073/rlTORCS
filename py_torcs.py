import lua
import gym
from gym import spaces
import numpy as np
from multiprocessing.managers import SyncManager
import inspect, os
from subprocess import Popen
import math

class MyManager(SyncManager):
    pass

MyManager.register("syncdict")

image_width = 160
image_height = 120

class TorcsEnv(gym.Env):
    '''
    sample config dict
    lg.opt = {'server': server,
              'game_config': game_config,
              'mkey': mkey,
              'auto_back': auto_back,
              'screen': screen}
    # some constants:
        use_RGB=True
    # what to control
        server=True
        auto_back=False
        game_config=fixed for now
        for each task: auto set
            unique mkey: 0,1,2,3...
            unique same screen
    '''
    metadata = {'render.modes': ['human', 'rgb_array']}
    def get_keys_to_action(self):
        keys = [ord('e'),ord('w'), ord('q'), ord('d'), ord('a'), ord('s'),ord('z')]
        keys_to_action = {}
        action_list = [0,1,2,3,5,6,7,8]
        for i,key in enumerate(keys):
            keys_to_action[tuple([key])] = action_list[i]
        #keys_to_action[tuple(keys)] = [0,1,2,3,5,6,7,8]

        return keys_to_action


    def print_slots(self, syncdict):
        print "occupied slots: ",
        for k in syncdict.keys():
            if syncdict.get(k):
                print k,

        print ""

    def allocate_id(self):
        #self.lock.acquire()
        self.id = -1
        for i in range(1000):
            if self.syncdict.get(i)==False:
                self.syncdict.update([(i, True)])
                self.id = i
                break
        print "after allocate: ",
        self.print_slots(self.syncdict)
        #self.lock.release()
        if self.id == -1:
            raise ValueError("all slots full")

    def free_id(self):
        #self.lock.acquire()
        assert(self.syncdict.get(self.id) == True)
        self.syncdict.update([(self.id, False)])
        self.id = -1
        print "after free: ",
        self.print_slots(self.syncdict)
        #self.lock.release()

    def __init__(self, subtype="discrete_improved", custom_reward="", detailed_info=False, **kwargs):
        print("initializing the first time")
        self.id = -1
        self.damage = 0
        self.custom_reward = custom_reward
        self.detailed_info = detailed_info

        # connect to the resource manager
        manager = MyManager(("127.0.0.1", 5000), authkey="password")
        manager.connect()
        self.syncdict = manager.syncdict()
        # decide not to use a lock

        # use the manager to get a valid id
        self.allocate_id()
        kwargs.update(mkey=self.id+200, screen=0)
        # set up the connection to the display
        os.environ['DISPLAY'] = ":"+str(self.id+100)
        self.xvfb = Popen(['Xvfb', os.environ['DISPLAY'], "-screen", "0", str(image_width)+"x"+str(image_height)+"x24", "+iglx"])
        self.suffix = str(self.id)

        self.sample_luaTable_type = type(lua.toTable({}))
    
        self.lg = lua.globals()
        # singleton design
        # the variables in TORCS.ctrl.cpp has not been append a suffix yet
        assert (self.lg.hasFirstInstance is None)
        self.lg.hasFirstInstance = True

        kwargs.update(use_RGB=True)
        setattr(self.lg, "opt"+self.suffix, lua.toTable(kwargs))     #self.lg.opt = lua.toTable(kwargs)
        self.subtype = subtype
        self.viewer = None

        # add the path to the lua path
        # current file directory
        currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
        self.lg.currentdir = ";" + currentdir + "/?.lua"
        self.lg.currentdir2 = ";" + currentdir + "/TORCS/?.so"
        lua.execute("package.path = package.path .. currentdir")
        lua.execute("package.cpath = package.cpath .. currentdir2")

        # another option is changing to the torcs working directory
        # os.chdir("../../rlTORCS")

        if subtype == "discrete":
            setattr(self.lg, "env_class" + self.suffix, lua.require("TORCS.TorcsDiscrete"))   #self.lg.env_class = lua.require("TORCS.TorcsDiscrete")
            lua.execute(" env"+self.suffix+" = env_class"+self.suffix+"(opt"+self.suffix+") ")
            self.action_space = spaces.Discrete(9)
        elif subtype == "discrete_improved":
            setattr(self.lg, "env_class" + self.suffix, lua.require("TORCS.TorcsDiscreteConstDamagePos"))   #self.lg.env_class = lua.require("TORCS.TorcsDiscreteConstDamagePos")
            lua.execute(" env" + self.suffix + " = env_class" + self.suffix + "(opt" + self.suffix + ") ") #lua.execute(" env = env_class(opt) ")
            self.action_space = spaces.Discrete(9)
        elif subtype == "continuous":
            raise NotImplemented("continuous subtype action has not been implemented")
        else:
            raise ValueError("invalid subtype of the environment, subtype = ", subtype)

        self.observation_space = spaces.Box(low=0, high=255, shape=(image_height, image_width, 3))

    def _reset(self):
        self.damage = 0
        lua.execute("env"+self.suffix+":kill()")
        obs = lua.eval("env"+self.suffix+":start()")
        return self._convert_obs(obs)

    def _step(self, u):
        assert self.action_space.contains(u)
        if self.subtype == "discrete" or self.subtype == "discrete_improved":
            # convert 0 based indexing to 1 based indexing in rlTorcs
            u += 1

        self.lg.u = u
        lua.execute("res1, res2, res3 = env"+self.suffix+":step(u)")
        reward, observation, terminal = self.lg.res1, self.lg.res2, self.lg.res3
        if self.detailed_info:
            sp = self.call_ctrl("getSpeed")
            angle = self.call_ctrl("getAngle")
            trackPos = self.call_ctrl("getPos")
            trackWidth = self.call_ctrl("getWidth")
            next_damage = self.call_ctrl("getDamage")
            is_stuck = lua.eval("env"+self.suffix+":isStuck()")
            #print('************* the damage number ***************')
            #print(self.damage)
            #print(next_damage)
            #print('***********************************************')
	    info = {'speed':sp, 'angle':angle, 'trackPos':trackPos, 'trackWidth':trackWidth, \
                   'damage':self.damage, 'next_damage':next_damage, 'is_stuck':is_stuck}
        if not(self.custom_reward == ""):
            reward = eval("self."+self.custom_reward+"()")
            if math.isnan(reward):
                reward = 0
                terminal = True
                print("Warning encountered reward == NaN!")
	if self.detailed_info:
            return self._convert_obs(observation), reward, terminal, info
        else:
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
        lua.execute(" env"+self.suffix+":cleanUp() ")
        self.xvfb.terminate()
        if self.id > 0:
            self.free_id()

    def __del__(self):
        self.close()

    # below reward engineering
    def reward_ben(self):
        sp = self.call_ctrl("getSpeed")
        angle = self.call_ctrl("getAngle")
        trackPos = self.call_ctrl("getPos")
        trackWidth = self.call_ctrl("getWidth")
        #print("pos", trackPos, "width", trackWidth)
        relativePos = trackPos / (trackWidth/2.0)
        next_damage = self.call_ctrl("getDamage")
        is_stuck = lua.eval("env"+self.suffix+":isStuck()")

        reward = sp * (np.cos(angle) - np.abs(np.sin(angle)) - np.abs(relativePos) )

        # collision detection
        if (next_damage - self.damage > 0) or is_stuck:
            reward = -10
        self.damage = next_damage

        reward = reward / 100.0 * 2.5
        return reward

    ######################## util functions ########################
    def _convert_obs(self, obs):
        obs = np.array(obs)
        # From 3*480*640 to 480*640*3
        obs = np.transpose(obs, [1, 2, 0])
        obs *= 255.0
        obs = obs.astype(np.uint8)
        return obs

    def _get_image(self):
        observation_RGB = np.zeros((3, image_height, image_width), dtype=np.float32)
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
        cmd = "env"+self.suffix+".ctrl."+func+"("
        n = len(args)

        for i in range(n):
            # push all args into the environment
            name = "arg"+str(i)
            var = self._convert_var(args[i])
            setattr(self.lg, name, var)
            # and construct the calling command
            cmd += name + ","

        if n > 0:
            cmd = cmd[:-1]
        cmd = cmd + ")"
        #print("calling command:", cmd)

        # multiple return results, not an issue here
        output = lua.eval(cmd)
        return self._convert_var(output)
