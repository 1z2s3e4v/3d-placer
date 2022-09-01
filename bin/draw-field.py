import pandas as pd
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.style as mplstyle
from matplotlib.collections import PatchCollection
from matplotlib import animation
import os
import glob
import re
import sys

class ShowPlace:
    def __init__(self, base_folder, plot_field=1):
        self.base_folder = base_folder
        self.plot_field = plot_field

        # search all records
        iter_paths = glob.glob(os.path.join(base_folder, "iter-*"))
        iter_folders = [path[len(base_folder):] for path in iter_paths]
        finder = re.compile(r"[0-9]+")
        iters = [int(finder.search(path).group(0)) for path in iter_folders]
        iters.sort()
        self.iters = iters
        print("found " + str(len(self.iters)) + " frames in " + base_folder)
        if(len(self.iters) == 0):
            return

        # load first frame
        if len(self.iters) == 0:
            raise
        else:
            self.load_data(self.iters[0])

        # init plt objects
        self.fig = plt.figure(figsize=(20, 10))

        # setup subplot
        ax = plt.subplot(1, 2, 1)
        if not self.plot_field:
            ax.set_xlim(self.die_info[0][0], self.die_info[0][1])
            ax.set_ylim(self.die_info[1][0], self.die_info[1][1])

        # draw cells in up layer
        self.up_cell_rects = []
        for cell in self.up_cell_pos:
            self.up_cell_rects.append(patches.Rectangle(
                (cell[0], cell[1]), cell[2], cell[3]))
        self.up_cell_patch_collection = PatchCollection(self.up_cell_rects, linewidth=1,
                                                        edgecolor="royalblue", facecolor="cornflowerblue", alpha=0.5)
        ax.add_collection(self.up_cell_patch_collection)

        # draw fillers in up layer
        self.up_filler_rects = []
        for cell in self.up_filler_pos:
            self.up_filler_rects.append(patches.Rectangle(
                (cell[0], cell[1]), cell[2], cell[3]))
        self.up_filler_patch_collection = PatchCollection(self.up_filler_rects, linewidth=1,
                                                          edgecolor="darkgrey", facecolor="lightgrey", alpha=0.5)
        ax.add_collection(self.up_filler_patch_collection)

        # draw electro field in up layer
        if self.plot_field:
            self.up_quiver = plt.quiver(
                self.up_x, self.up_y, self.up_electro_x, self.up_electro_y, color="red")

        # set up layer title
        plt.title("up layer")

        # setup subplot
        ax = plt.subplot(1, 2, 2)
        if not self.plot_field:
            ax.set_xlim(self.die_info[0][0], self.die_info[0][1])
            ax.set_ylim(self.die_info[1][0], self.die_info[1][1])

        # draw cells in down layer
        self.down_cell_rects = []
        for cell in self.down_cell_pos:
            self.down_cell_rects.append(patches.Rectangle(
                (cell[0], cell[1]), cell[2], cell[3]))
        self.down_cell_patch_collection = PatchCollection(self.down_cell_rects, linewidth=1,
                                                          edgecolor="royalblue", facecolor="cornflowerblue", alpha=0.5)
        ax.add_collection(self.down_cell_patch_collection)

        # draw fillers in down layer
        self.down_filler_rects = []
        for cell in self.down_filler_pos:
            self.down_filler_rects.append(patches.Rectangle(
                (cell[0], cell[1]), cell[2], cell[3]))
        self.down_filler_patch_collection = PatchCollection(self.down_filler_rects, linewidth=1,
                                                            edgecolor="darkgrey", facecolor="lightgrey", alpha=0.5)
        ax.add_collection(self.down_filler_patch_collection)

        # draw electro field in down layer
        if self.plot_field:
            self.down_quiver = plt.quiver(
                self.down_x, self.down_y, self.down_electro_x, self.down_electro_y, color="red")

        # set down layer title
        plt.title("down layer")

        # set super title
        self.sup_title = plt.suptitle("Iter " + str(self.iters[0]))

        # build animation
        self.anim = animation.FuncAnimation(
            self.fig, self.update_frame, frames=len(self.iters), interval=150, repeat_delay=3000, repeat=False, blit=False)

    def load_data(self, iter):
        iter_prefix = "iter-" + str(iter)

        self.die_info = np.genfromtxt(os.path.join(
            self.base_folder, iter_prefix, "die.csv"), delimiter=',')

        if self.plot_field:
            self.up_electro_x = np.genfromtxt(os.path.join(
                self.base_folder, iter_prefix, "field-up-x.csv"), delimiter=',')
            self.up_electro_y = np.genfromtxt(os.path.join(
                self.base_folder, iter_prefix, "field-up-y.csv"), delimiter=',')
            self.up_x, self.up_y = np.meshgrid(np.linspace(
                self.die_info[0][0], self.die_info[0][1], self.up_electro_x.shape[1]), np.linspace(self.die_info[1][0], self.die_info[1][1], self.up_electro_y.shape[0]))

            self.down_electro_x = np.genfromtxt(os.path.join(
                self.base_folder, iter_prefix, "field-down-x.csv"), delimiter=',')
            self.down_electro_y = np.genfromtxt(os.path.join(
                self.base_folder, iter_prefix, "field-down-y.csv"), delimiter=',')
            self.down_x, self.down_y = np.meshgrid(np.linspace(
                self.die_info[0][0], self.die_info[0][1], self.down_electro_x.shape[1]), np.linspace(self.die_info[1][0], self.die_info[1][1], self.down_electro_y.shape[0]))

        self.up_cell_pos = np.genfromtxt(os.path.join(
            self.base_folder, iter_prefix, "cell-up.csv"), delimiter=',')
        self.up_filler_pos = np.genfromtxt(os.path.join(
            self.base_folder, iter_prefix, "filler-up.csv"), delimiter=',')
        self.down_cell_pos = np.genfromtxt(os.path.join(
            self.base_folder, iter_prefix, "cell-down.csv"), delimiter=',')
        self.down_filler_pos = np.genfromtxt(os.path.join(
            self.base_folder, iter_prefix, "filler-down.csv"), delimiter=',')
        self.hb_pos = np.genfromtxt(os.path.join(
            self.base_folder, iter_prefix, "hb.csv"), delimiter=',')

    def update_frame(self, frame_num):
        # load data in this frame
        self.load_data(self.iters[frame_num])

        # update cell pos in up layer
        for i, rect in enumerate(self.up_cell_rects):
            rect.set_xy((self.up_cell_pos[i][0], self.up_cell_pos[i][1]))
        self.up_cell_patch_collection.set_paths(self.up_cell_rects)

        # update filler pos in up layer
        for i, rect in enumerate(self.up_filler_rects):
            rect.set_xy((self.up_filler_pos[i][0], self.up_filler_pos[i][1]))
        self.up_filler_patch_collection.set_paths(self.up_filler_rects)

        # update quiver in up layer #########
        if self.plot_field:
            self.up_quiver.set_UVC(self.up_electro_x, self.up_electro_y)

        # update cell pos in down layer
        for i, rect in enumerate(self.down_cell_rects):
            rect.set_xy((self.down_cell_pos[i][0], self.down_cell_pos[i][1]))
        self.down_cell_patch_collection.set_paths(self.down_cell_rects)

        # update filler pos in down layer
        for i, rect in enumerate(self.down_filler_rects):
            rect.set_xy(
                (self.down_filler_pos[i][0], self.down_filler_pos[i][1]))
        self.down_filler_patch_collection.set_paths(self.down_filler_rects)

        # update quiver in down layer
        if self.plot_field:
            self.down_quiver.set_UVC(self.down_electro_x, self.down_electro_y)

        # set super title
        self.sup_title.set_text("Iter " + str(self.iters[frame_num]))

        return None

if(len(sys.argv)<2):
    print('no argument')
    sys.exit()
caseName = sys.argv[1]
PLOT_FIELD = 1
PLOT_FILLER = 1
if len(sys.argv)>2:
    PLOT_FIELD = int(sys.argv[2])
if len(sys.argv)>3:
    PLOT_FILLER = int(sys.argv[3])
#BASE_FOLDER = "./draw/"+caseName+"/plot"
GIF_PREFIX = "./draw/"+caseName+"/"+caseName

# fo_viewer = ShowPlace("./plot/eplace/filler-only", plot_field=PLOT_FIELD)
# fo_viewer.anim.save(GIF_PREFIX + "-filler-only.gif")
#plt.show(block=False)

if(PLOT_FILLER):
    mj_viewer = ShowPlace("./draw/"+caseName+"/plot/plot-filler", plot_field=False)
    mj_viewer.anim.save(GIF_PREFIX + "-filler.gif")

mj_viewer = ShowPlace("./draw/"+caseName+"/plot", plot_field=PLOT_FIELD)
mj_viewer.anim.save(GIF_PREFIX + "-major.gif")
#plt.show(block=False)

