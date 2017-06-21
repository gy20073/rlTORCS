from gym.envs.registration import register
import os
print("before running register")
register(
    id='rltorcs-v0',
    entry_point='rltorcs.envs:TorcsEnv',
    kwargs={subtype: "discrete_improved",
            server: True,
            auto_back: False,
            game_config: os.path.abspath('../game_config/quickrace_discrete_single.xml')}
)
