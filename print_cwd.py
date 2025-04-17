import os, shutil
from pathlib import Path
# Printing dir tree:
print(f"{os.getcwd()=}")
for root, dirs, files in os.walk(os.getcwd()):
    print("Directory:", root)

    print("Subdirectories:")
    for dir in dirs:
        print(f"\t{dir}")

    print("Files:")
    for file in files:
        print(f"\t{file}")

print("-" * 20)