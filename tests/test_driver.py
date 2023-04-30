import subprocess
import sys
import os

cc_files = [file for file in os.listdir("tests/") if file.endswith(".cc")]

for cc_file in cc_files:
    exe_file = "build/" + cc_file + ".out"
    print(" ".join(sys.argv[1:] + ["-lu", "-Lbuild/", "tests/" + cc_file, "-o", exe_file]))
    compile_process = subprocess.run(sys.argv[1:] + ["tests/" + cc_file, "-o", exe_file, "-lu", "-Lbuild/"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if compile_process.returncode != 0:
        print("Compile for", cc_file, "failed.")
        continue
    test_process = subprocess.run(exe_file, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if test_process.returncode != 0:
        print("Test", cc_file, "failed.")
        continue
    print("Test", cc_file, "succeeded.")
