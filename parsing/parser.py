import F9PReader
from argparse import ArgumentParser
import IMUReader
import numpy as np
from matplotlib import pyplot as plt
from pathlib import Path
import visualization as vis


NOMINAL_INTERVAL_JUMP_SAMPLES = 64
IMU_SAMPLING_FREQ_HZ = 1600
NOMINAL_INTERVAL_JUMP_NS = (NOMINAL_INTERVAL_JUMP_SAMPLES * 1e9) // IMU_SAMPLING_FREQ_HZ


def get_paths_from_args():
    parser = ArgumentParser(description="Parse and process tracker-box data.")
    parser.add_argument(
        "data_dir",
        type=Path,
        help='Parent data directory for raw and parsed data. Searches for/creates subdirectories "raw" and "parsed".',
    )
    parser.add_argument(
        "file_nr",
        type=int,
        help='Integer identifier of data files. Assumes files are named "IMU-[n].bin", "F9P0-[n].bin" and "F9P0-[n].bin" (.pkl for parsed files)',
    )
    args = parser.parse_args()

    raw_dir = args.data_dir / "raw"
    parsed_dir = args.data_dir / "parsed"
    parsed_dir.mkdir(parents=False, exist_ok=True)

    IMU_fname = raw_dir / f"IMU-{args.file_nr}.bin"
    F9P0_fname = raw_dir / f"F9P0-{args.file_nr}.bin"
    F9P1_fname = raw_dir / f"F9P1-{args.file_nr}.bin"
    IMU_parsed_fname = parsed_dir / f"IMU-{args.file_nr}.pkl"

    return IMU_fname, F9P0_fname, F9P1_fname, IMU_parsed_fname


def sync_times(stamped, times):
    # Find indices of stamped IMU samples and remove samples before first and after last stamp
    stamped_idcs = np.nonzero(stamped)[0]
    start_idx = stamped_idcs[0]
    end_idx = stamped_idcs[-1] + 1
    stamped = stamped[start_idx:end_idx]
    stamped_idcs = np.nonzero(stamped)[0]

    # Find first positive change in interval between timestamps
    # For IMU samples:
    d_stamp_intervals = np.diff(np.diff(stamped_idcs))
    first_peak = (d_stamp_intervals > 0).argmax()
    # For GNSS timestamps:
    d_time_intervals = np.diff(np.diff(times))
    first_time_peak = (d_time_intervals > 0).argmax()

    if d_stamp_intervals[first_peak] != NOMINAL_INTERVAL_JUMP_SAMPLES:
        print(
            f"WARNING: Stamp interval jump of {d_stamp_intervals[first_peak]} samples found. {NOMINAL_INTERVAL_JUMP_SAMPLES} samples was expected."
        )
    if abs(d_time_intervals[first_time_peak] - NOMINAL_INTERVAL_JUMP_NS) > 1e6:
        print(
            f"WARNING: Stamp interval jump of {d_time_intervals[first_time_peak]} ns found. {NOMINAL_INTERVAL_JUMP_NS} ns was expected."
        )

    offset = first_time_peak - first_peak
    print(f"Found offset of {offset} stamps.")

    # Remove initial IMU samples or GNSS timestamps to line up the two time series
    if offset > 0:
        times = times[offset:]
    else:
        idx_offset = stamped_idcs[-offset]
        start_idx += idx_offset
        stamped = stamped[idx_offset:]
        stamped_idcs = np.nonzero(stamped)[0]

    # Linear interpolation between timestamps
    interp_times = np.zeros(end_idx - start_idx, dtype=np.uint64)
    interp_idx = 0
    for dx, dy, prev_y in zip(np.diff(stamped_idcs), np.diff(times), times[:-1]):
        for i in range(dx):
            interp_times[interp_idx] = prev_y + (i * dy) // dx
            interp_idx += 1
    interp_times[-1] = times[-1]
    assert end_idx - start_idx == len(interp_times)

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
    t, acc, gyr = IMUReader.load_parsed(IMU_parsed_fname)
    angles = IMUReader.estimate_euler(acc, gyr, IMU_SAMPLING_FREQ_HZ)

    vis.playback(angles, IMU_SAMPLING_FREQ_HZ, fname="output.mp4")


if __name__ == "__main__":
    main()
