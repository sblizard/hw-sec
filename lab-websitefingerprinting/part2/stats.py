import json
import numpy as np
import math


with open("../part3/tracespart24.out", "r") as f:
    data = json.loads(f.read())

    traces = np.array(data["traces"])

    for i in range(0, len(traces), 4):
        trace_batch = traces[i : i + 4]
        print(f"---Data for Traces {i} to {i+3}---")
        print(f"Max: {trace_batch.max()}")
        print(f"Min: {trace_batch.min()}")
        print(f"Mean: {trace_batch.mean()}")
        print(f"Median: {np.median(trace_batch)}\n")
