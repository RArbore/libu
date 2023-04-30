import subprocess
import sys
import os

cc_files = [file for file in os.listdir("tests/") if file.endswith(".cc")]
succeeded = 0

for i, cc_file in enumerate(cc_files):
    exe_file = "build/" + cc_file + ".out"
    compile_process = subprocess.run(sys.argv[1:] + ["tests/" + cc_file, "-o", exe_file, "-lu", "-Lbuild/"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if compile_process.returncode != 0:
        status = 0
    else:
        test_process = subprocess.run(exe_file, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if test_process.returncode != 0:
            status = 1
        else:
            status = 2
    
    if status == 0:
        print("[{}/{}] Test didn't compile.\t\033[91mFAIL\033[00m".format(i+1, len(cc_files)))
    elif status == 1:
        print("[{}/{}] Test didn't pass.\t\033[91mFAIL\033[00m".format(i+1, len(cc_files)))
    else:
        print("[{}/{}] Test passed.\t\033[92mSUCCESS\033[00m".format(i+1, len(cc_files)))
        succeeded += 1

if succeeded < len(cc_files):
    print("\033[91m{}/{}\033[00m tests passed.".format(succeeded, len(cc_files)))
else:
    print("\033[92m{}/{}\033[00m tests passed.".format(succeeded, len(cc_files)))
