import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from typing import List
from tabulate import tabulate
from collections import defaultdict
import seaborn as sns

def lighten_color(color, alpha=0.4):
    c = mcolors.to_rgb(color)  # Convert to RGB tuple
    return tuple(c[i] + (1 - c[i]) * alpha for i in range(3))


class Analyser:
    def __init__(self, data_file, meta_data_file, type=""):
        self.data_file = data_file
        self.meta_data_file = meta_data_file
        self.get_data(data_file, meta_data_file)
        self.type = type
        self.colors = ["#1f77b4","#ff7f0e","#2ca02c","#9467bd","#8c564b","#e377c2","#7f7f7f","#bcbd22","#17becf",]
        self.context_switch_overhead = 0
        self.avg_response_time = 0
        
    def get_data(self, data_file, meta_data_file):
        self.data = pd.read_csv(data_file, header=None, names=["time", "thread"])
        self.thread_info_df = pd.read_csv(meta_data_file)
        self.thread_info = self.thread_info_df.to_dict("records")

    def plot_gantt_chart(self):
        data = self.data
        thread_info = self.thread_info
        t = data["time"]
        p = data["thread"]

        bars = []
        n = len(p)

        start = t[0]

        for i in range(1, n):
            if p[i] != p[i - 1]:
                bars.append((start, t[i - 1], p[i - 1]))
                start = t[i]
            if i == n - 1:
                bars.append((start, t[i], p[i]))

        i = 0
        colors = self.colors
        thread_colors = {}
        for i, el in enumerate(thread_info):
            id = el["id"]
            thread_colors[id] = colors[i % len(colors)]
        thread_colors[-1] = "#d62728"

        fig, ax = plt.subplots(figsize=(8, 2.5))
        i = 0
        for el in bars:
            ax.broken_barh(
                [(el[0], el[1] - el[0])],  # (start_time, duration)
                (-0.4, 0.8),  # (y_position, bar_height)
                facecolors=thread_colors[el[2]],  # cycling colors
            )
            # Add text label at the center of the bar
            ax.text(
                x=el[0] + (el[1] - el[0]) / 2,  # Center of the bar along time axis
                y=0,  # Y-axis position (same as bar's center)
                s=el[2] if (el[2] != -1) else "x",  # Text to display
                va="center",  # Vertical alignment
                ha="center",  # Horizontal alignment
                color="black",  # Text color
                fontsize=12,  # Optional: text size
            )
            i = i + 1

        # Labels and grid
        ax.set_xlabel("Time (ms)")
        ax.set_ylabel("Processes")
        ax.set_yticks([])
        ax.set_ylim(-1, 1)
        # ax.set_yticklabels([f'P{process["id"]}' for process in processes])

        plt.title(self.type + " Gantt Chart")
        plt.show()

    def plot_individual_gantt_chart(self):
        data = self.data
        thread_info = self.thread_info
        t = data["time"]
        p = data["thread"]

        bars = []
        n = len(p)

        start = t[0]

        for i in range(1, n):
            if p[i] != p[i - 1]:
                bars.append((start, t[i - 1], p[i - 1]))
                start = t[i]
            if i == n - 1:
                bars.append((start, t[i], p[i]))

        colors = self.colors
        
        i = 0

        thread_colors = {}
        for i, el in enumerate(thread_info):
            id = el["id"]
            thread_colors[id] = colors[i % len(colors)]
        thread_colors[-1] = "#d62728"

        fig, ax = plt.subplots(figsize=(8, 2.5))

        for el in bars:
            ax.broken_barh(
                [(el[0], el[1] - el[0])],  # (start_time, duration)
                (-0.4, 0.8),  # (y_position, bar_height)
                facecolors=thread_colors[el[2]],  # cycling colors
            )
            # Add text label at the center of the bar
            ax.text(
                x=el[0] + (el[1] - el[0]) / 2,  # Center of the bar along time axis
                y=0,  # Y-axis position (same as bar's center)
                s=el[2] if (el[2] != -1) else "x",  # Text to display
                va="center",  # Vertical alignment
                ha="center",  # Horizontal alignment
                color="black",  # Text color
                fontsize=12,  # Optional: text size
            )
            i = i + 1

        i = 1
        for x in thread_info:
            clr = thread_colors[x["id"]]
            lighter_clr = lighten_color(clr, alpha=0.5)
            ax.broken_barh(
                [
                    (x["arrival_time"], x["completion_time"] - x["arrival_time"])
                ],  # (start_time, duration)
                (i - 0.4, 0.8),  # (y_position, bar_height)
                facecolors=lighter_clr,  # cycling colors
            )
            for el in bars:
                if el[2] != x["id"]:
                    continue
                ax.broken_barh(
                    [(el[0], el[1] - el[0])],  # (start_time, duration)
                    (i - 0.4, 0.8),  # (y_position, bar_height)
                    facecolors=thread_colors[el[2]],  # cycling colors
                )
                # Add text label at the center of the bar
                ax.text(
                    x=-10,  # Center of the bar along time axis
                    y=i,  # Y-axis position (same as bar's center)
                    s=el[2] if (el[2] != -1) else "x",  # Text to display
                    va="center",  # Vertical alignment
                    ha="center",  # Horizontal alignment
                    color="black",  # Text color
                    fontsize=10,  # Optional: text size
                )
            i = i + 1

        # Labels and grid
        ax.set_xlabel("Time (ms)")
        ax.set_ylabel("Processes")
        ax.set_yticks([])
        plt.title(self.type + " individual Gantt Chart")
        plt.show()

    def evaluate_performance(self, print_output=True):
        thread_info = self.thread_info
        avg_waiting_time = 0
        avg_burst_time = 0
        max_waiting_time = -float("inf")
        
        for el in thread_info:
            avg_waiting_time += el["waiting_time"]
            avg_burst_time += el["burst_time"]
            max_waiting_time = max(max_waiting_time, el["waiting_time"])
        avg_waiting_time /= len(thread_info)
        avg_burst_time /= len(thread_info)
        self.avg_waiting_time = avg_waiting_time
        self.max_waiting_time = max_waiting_time
        self.avg_burst_time = avg_burst_time
        
        self.calculate_switches()
        self.calculate_response_time()
        if(not print_output) :
            return
        print("Average waiting time of the threads = ", avg_waiting_time, "ms")
        print("Peak waiting time of the threads = ", max_waiting_time, "ms")
        print("Average burst time of the threads = ", avg_burst_time, "ms")
        print()
        print("Average Response Time = ", self.avg_response_time, " ms")
        print("Context Switch Overhead = ", self.context_switch_overhead)

    def calculate_switches(self):
        self.context_switch_overhead = 0
        p = self.data["thread"]
        for i in range(1,len(p)):
            self.context_switch_overhead  += 1 if(p[i]!=p[i-1]) else 0
    
    def calculate_response_time(self):
        seen = defaultdict(bool)
        t = self.data["time"]
        p = self.data["thread"]
        self.avg_response_time = 0
        for i in range(len(p)):
            if p[i] ==-1:
                continue
            if seen[p[i]]:
                continue
            
            for el in self.thread_info:
                if el['id']==p[i]:
                    self.avg_response_time+=(t[i]-el['arrival_time'])
            seen[p[i]] = True
            
        self.avg_response_time/=len(self.thread_info)
        
        
    def show_thread_info(self):
        print(self.thread_info_df)
        

def compare_schedulers(l: List[Analyser], plot=True):
    df = pd.DataFrame(columns=["Scheduling Algo", "Average Waiting Time", "Peak Waiting Time", "Average Response Time", "Context Switching Overhead"])
    for obj in l:
        obj.evaluate_performance(False)
        df.loc[len(df)] = [obj.type, obj.avg_waiting_time, obj.max_waiting_time, obj.avg_response_time, obj.context_switch_overhead]
        
    print(tabulate(df, headers='keys', tablefmt='pretty', showindex=False))
    
    if plot:
        # Optional: set seaborn style for cleaner plots
        sns.set_style("whitegrid")

        for col in df.select_dtypes(include='number').columns:
            plt.figure(figsize=(6, 4))  # Slightly larger for readability

            # Bar plot with color and edge style
            bars = plt.bar(df["Scheduling Algo"], df[col], color='#1f77b4')

            # Add values on top of bars
            for bar in bars:
                height = bar.get_height()
                plt.text(bar.get_x() + bar.get_width() / 2, height + 0.01 * height,
                        f"{height:.2f}", ha='center', va='bottom', fontsize=8)

            plt.xlabel("Scheduling Algorithm", fontsize=10)
            plt.ylabel(col, fontsize=10)
            plt.title(f"{col}", fontsize=12)
            plt.xticks(rotation=45, ha='right', fontsize=9)
            plt.yticks(fontsize=9)
            plt.tight_layout()
            # plt.grid(axis='y', linestyle='--', alpha=0.6)
            plt.grid(False)
            plt.show()