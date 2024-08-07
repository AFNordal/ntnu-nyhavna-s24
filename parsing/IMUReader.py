import struct
import numpy as np
from ahrs.filters.madgwick import Madgwick
from ahrs.common.quaternion import Quaternion
import pickle


IMU_sample_type = np.dtype(
    [
        ("time", np.uint64),
        ("acc", np.float32, 3),
        ("gyr", np.float32, 3),
    ]
)


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


def load_IMU_data(fname):
    with open(fname, "rb") as file:
        contents = file.read()
    raw_data = np.array(
        [
            i[0:8]
            for i in struct.iter_unpack(
                "<" + "h" * 6 + "I" + "B" * 4 + "I" * 3, contents
            )
        ]
    )
    idx = raw_data[:, 6]
    stamped = np.bool(raw_data[:, 7])
    acc = raw_data[:, 0:3] * 4 / (2**16) * 9.80665
    gyr = raw_data[:, 3:6] * 4000 / (2**16) * np.pi / 180


    return stamped, acc, gyr


def save_parsed(fname, time, acc, gyr):
    arr = np.array(
        [(t, a, g) for t, a, g in zip(time, acc, gyr)],
        dtype=IMU_sample_type,
    )
    with open(fname, "wb") as f:
        pickle.dump(arr, f)


def load_parsed(fname):
    with open(fname, "rb") as f:
        arr = pickle.load(f)
    time = arr[:]["time"]
    acc = arr[:]["acc"]
    gyr = arr[:]["gyr"]
    return time, acc, gyr

