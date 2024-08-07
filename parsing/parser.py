import F9PReader
from argparse import ArgumentParser
import IMUReader
import numpy as np
from matplotlib import pyplot as plt
from pathlib import Path
import visualization as vis


def get_paths_from_args():
    parser = ArgumentParser()
    parser.add_argument("data_dir", type=Path)
    parser.add_argument("file_nr", type=int)
    args = parser.parse_args()
    IMU_fname = args.data_dir / f"IMU-{args.file_nr}.bin"
    F9P0_fname = args.data_dir / f"F9P0-{args.file_nr}.bin"
    F9P1_fname = args.data_dir / f"F9P1-{args.file_nr}.bin"
    IMU_parsed_fname = args.data_dir / f"IMU-{args.file_nr}.pkl"
    return IMU_fname, F9P0_fname, F9P1_fname, IMU_parsed_fname


def sync_times(stamped, times):


    n_samples = len(stamped)
    stamped_idcs = np.nonzero(stamped)[0]
    start_idx = stamped_idcs[0]
    end_idx = stamped_idcs[-1] + 1

    stamped = stamped[start_idx:end_idx]
    stamped_idcs = np.nonzero(stamped)[0]

    d_stamp_intervals = np.diff(np.diff(stamped_idcs))
    first_peak = (d_stamp_intervals > 0).argmax()

    d_time_intervals = np.diff(np.diff(times))
    first_time_peak = (d_time_intervals > 0).argmax()

    offset = first_time_peak - first_peak

    print(f"Found offset of {offset} stamps.")

    if offset > 0:
        times = times[offset:]
    else:
        idx_offset = stamped_idcs[-offset]
        start_idx += idx_offset
        stamped = stamped[idx_offset:]
        stamped_idcs = np.nonzero(stamped)[0]

    # Interpolation
    interp_times = np.zeros(end_idx-start_idx, dtype=np.uint64)
    interp_idx = 0
    for dx, dy, prev_y in zip(np.diff(stamped_idcs), np.diff(times), times[:-1]):
        for i in range(dx):
            interp_times[interp_idx] = prev_y + (i * dy) // dx
            interp_idx += 1
    interp_times[-1] = times[-1]
    assert(end_idx-start_idx == len(interp_times))
    return interp_times, start_idx, end_idx


def main():
    IMU_fname, F9P0_fname, F9P1_fname, IMU_parsed_fname = get_paths_from_args()

    stamped, acc, gyr = IMUReader.load_IMU_data(IMU_fname)
    F9P1_msgs = F9PReader.parse_file(F9P1_fname)
    F9P1_times = F9PReader.get_timestamps(F9P1_msgs)

    sample_times, start_idx, end_idx = sync_times(stamped, F9P1_times)
    acc = acc[start_idx:end_idx]
    gyr = gyr[start_idx:end_idx]
    
    IMUReader.save_parsed(IMU_parsed_fname, sample_times, acc, gyr)
    angles = IMUReader.estimate_euler(acc, gyr)

    vis.playback(angles)
    
    
    

if __name__ == "__main__":
    main()
