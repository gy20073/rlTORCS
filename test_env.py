#!/usr/bin/python

import py_torcs
import random

env = py_torcs.TorcsEnv("discrete", server=True)
env.reset()
for i in range(3000):
	#env.step(random.randint(0, 8))
	env.step(1)
	#env._render()

env.close()

