import os

def check_path(path):
    if os.path.isdir(path):
        print("[%s] CHECKED (dir)" % path)
        return True
    elif os.path.exists(path):
        print("[%s] CHECKED (file)" % path)
        return True
    else:
        print("ERROR:: [%s] Files or directories are NOT found!!" % path)
        return False