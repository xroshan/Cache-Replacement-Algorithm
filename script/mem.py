import random

with open("t.dat", "w") as f:

    for i in range(5000):
        f.write(f"{i} {random.randint(0, 1000)} {random.randint(0,1)}\n")
