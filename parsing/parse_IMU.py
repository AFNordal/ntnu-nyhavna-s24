from argparse import ArgumentParser
import struct
import numpy as np
from matplotlib import pyplot as plt
from tqdm import tqdm

def get_fname():
    parser = ArgumentParser()
    parser.add_argument("file")
    args = parser.parse_args()
    return args.file

def main():
    fname = get_fname()
    with open(fname, "rb") as file:
        contents = file.read()
    data = np.array([i for i in tqdm(struct.iter_unpack("<"+"h"*6+"I"+"B"*4+"I"*3, contents))])
    plt.plot(data[:, 6], data[:, 0])
    plt.show()
        # for sample in data:
        #     ax, ay, az, gx, gy, gz, j, f, _, _, _, _, _, _ = sample




if __name__ == "__main__":
    main()