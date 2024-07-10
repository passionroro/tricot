import matplotlib.pyplot as plt
import numpy as np

def normalize_rgb(rgb_values):
    rgb_array = np.array(rgb_values)
    if rgb_array.shape[1] == 4:
        rgb_array = rgb_array[:, :3]
    return rgb_array / 255.0

def visualize_colors(rgb_values, name):
    fig, ax = plt.subplots(figsize=(10, 2))
    
    # Create color patches
    for i, color in enumerate(rgb_values):
        ax.add_patch(plt.Rectangle((i, 0), 1, 1, color=color))
    
    # Set the limits and remove axes
    ax.set_xlim(0, len(rgb_values))
    ax.set_ylim(0, 1)
    ax.axis('off')
    
    plt.title(name)
    plt.tight_layout()
    plt.show()

# Example usage
if __name__ == "__main__":
    hist_colors = [
        [168, 105, 105],
        [71, 91, 104],
        [119, 152, 125],
        [232, 206, 166],
        [94, 124, 168],
        [96, 90, 81],
        [112, 165, 200],
        [176, 110, 163]
    ]
    visualize_colors(normalize_rgb(hist_colors), 'Hist')