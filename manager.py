from multiprocessing.managers import SyncManager
import time
from threading import Lock

class MyManager(SyncManager):
    pass

max_clients=512
syncdict = {}
for i in range(max_clients):
    syncdict[i] = False

def get_dict():
    return syncdict

def print_slots(syncdict):
    print "occupied slots: ",
    for k in syncdict.keys():
        if syncdict[k]:
            print k,

    print ""

if __name__ == "__main__":
    MyManager.register("syncdict", get_dict)
    manager = MyManager(("127.0.0.1", 5000), authkey="password")
    manager.start()
    while True:
        print_slots(syncdict)
        time.sleep(10)
