import numpy as np
from matplotlib import pyplot as plt
import matplotlib
import matplotlib.animation as animation

matplotlib.use("tkagg")


def plot_acc_gyr(idx, acc, gyr):
    fig, (a1, a2) = plt.subplots(2, 1, sharex=True)
    a1.plot(idx, acc[:, 0])
    a1.plot(idx, acc[:, 1])
    a1.plot(idx, acc[:, 2])
    a1.legend(("x", "y", "z"))
    a1.title.set_text("accelerometer data")
    a2.plot(idx, gyr[:, 0])
    a2.plot(idx, gyr[:, 1])
    a2.plot(idx, gyr[:, 2])
    a2.legend(("x", "y", "z"))
    a2.title.set_text("gyroscope data")
    plt.show()


def plot_ang(idx, ang):
    plt.plot(idx, ang[:, 0])
    plt.plot(idx, ang[:, 1])
    plt.plot(idx, ang[:, 2])
    plt.title("estimated euler angles")
    plt.show()


def rot_x(theta):
    return np.array(
        [
            [1, 0, 0],
            [0, np.cos(theta), -np.sin(theta)],
            [0, np.sin(theta), np.cos(theta)],
        ]
    )


def rot_y(theta):
    return np.array(
        [
            [np.cos(theta), 0, np.sin(theta)],
            [0, 1, 0],
            [-np.sin(theta), 0, np.cos(theta)],
        ]
    )


def rot_z(theta):
    return np.array(
        [
            [np.cos(theta), -np.sin(theta), 0],
            [np.sin(theta), np.cos(theta), 0],
            [0, 0, 1],
        ]
    )


def rotmat_from_euler(euler):
    return rot_z(euler[2]) @ rot_y(euler[1]) @ rot_x((euler[0]))


def playback(ang):
    figure = plt.figure()
    ax = figure.add_subplot(projection="3d")
    colors = ["k", "r", "g", "b", "b", "b"]
    vecs0 = np.array(
        [
            [-1, 0, 0],
            [-1, 0, 0],
            [1, 0, 0],
            [0, 0, 0],
            [0, 0.5, 0],
            [0, 0.5, 0],
        ]
    )  # row vectors
    vecs1 = np.array(
        [
            [1, 0, 0],
            [-1, 0, 0.5],
            [1, 0, 0.5],
            [0, 0.5, 0],
            [0.2, 0.3, 0],
            [-0.2, 0.3, 0],
        ]
    )  # row vectors
    lines = []
    for i in range(len(vecs0)):
        lines.append(
            ax.plot(
                (vecs0[i, 0], vecs1[i, 0]),
                (vecs0[i, 1], vecs1[i, 1]),
                (vecs0[i, 2], vecs1[i, 2]),
                colors[i],
            )[0]
        )
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2, 2)
    ax.set_zlim(-2, 2)
    ax.set_aspect("equal")

    def animate(i):
        rotmat = rotmat_from_euler(ang[i * 100])
        rotvecs0 = rotmat @ vecs0.T
        rotvecs1 = rotmat @ vecs1.T
        for j in range(len(vecs0)):
            lines[j].set_data_3d(
                (rotvecs0[0, j], rotvecs1[0, j]),
                (rotvecs0[1, j], rotvecs1[1, j]),
                (rotvecs0[2, j], rotvecs1[2, j]),
            )
        return lines

    anim = animation.FuncAnimation(
        fig=figure,
        func=animate,
        frames=len(ang) // 100,
        interval=1000 // 16,
    )
    plt.show()

