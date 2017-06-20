from multiprocessing.managers import SyncManager
import time
from threading import Lock

class MyManager(SyncManager):
    pass

syncdict = {}
for i in range(15):
    syncdict[i] = False

def get_dict():
    return syncdict

if __name__ == "__main__":
    MyManager.register("syncdict", get_dict)
    manager = MyManager(("127.0.0.1", 5000), authkey="password")
    manager.start()
    while True:
        print manager.syncdict()
        time.sleep(3)
