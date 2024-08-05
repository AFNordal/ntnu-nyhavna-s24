from argparse import ArgumentParser
import struct
import numpy as np
from ahrs.filters.madgwick import Madgwick
from ahrs.common.quaternion import Quaternion
from visualization import *
import pickle


IMU_sample_type = np.dtype(
    [
        ("timestamp", np.uint64),
        ("acc", np.float32, 3),
        ("gyr", np.float32, 3),
    ]
)


def get_fname():
    parser = ArgumentParser()
    parser.add_argument("file")
    args = parser.parse_args()
    return args.file


def estimate_euler(acc, gyr):
    IMUfilter = Madgwick(gyr, acc, frequency=1600, var_acc=0.001, var_gyr=4e-06)
    ang = np.array([Quaternion(q).to_angles() for q in IMUfilter.Q])
    return ang


def print_variances(gyr, acc):
    print(
        f"""Accelerometer:
X: {np.var(acc[:, 0])}
Y: {np.var(acc[:, 1])}
Z: {np.var(acc[:, 2])}

Gyroscope:
X: {np.var(gyr[:, 0])}
Y: {np.var(gyr[:, 1])}
Z: {np.var(gyr[:, 2])}"""
    )


def load_acc_gyr(fname):
    with open(fname, "rb") as file:
        contents = file.read()
    raw_data = np.array(
        [
            i[0:7]
            for i in struct.iter_unpack(
                "<" + "h" * 6 + "I" + "B" * 4 + "I" * 3, contents
            )
        ]
    )
    idx = raw_data[:, 6]
    acc = raw_data[:, 0:3] * 4 / (2**16) * 9.80665
    gyr = raw_data[:, 3:6] * 4000 / (2**16) * np.pi / 180

    return idx, acc, gyr


def save_parsed(fname, acc, gyr):
    arr = np.array(
        [(0, a, g) for a, g in zip(acc, gyr)],
        dtype=IMU_sample_type,
    )
    with open(fname, "wb") as f:
        pickle.dump(arr, f)


def load_parsed(fname):
    with open(fname, "rb") as f:
        arr = pickle.load(f)
    acc = arr[:]["acc"]
    gyr = arr[:]["gyr"]
    return acc, gyr


def main():
    fname = get_fname()
    idx, acc, gyr = load_acc_gyr(fname)
    save_parsed(fname.split(".")[0] + ".pkl", acc, gyr)
    acc, gyr = load_parsed(fname.split(".")[0] + ".pkl")
    plot_acc_gyr(list(range(len(acc))), acc, gyr)
    playback(estimate_euler(acc, gyr))


if __name__ == "__main__":
    main()
