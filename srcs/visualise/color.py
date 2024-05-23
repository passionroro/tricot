import matplotlib.pyplot as plt
import numpy as np

def normalize_rgb(rgb_values):
	return np.array(rgb_values) / 255.0

def visualize_colors(rgb_values):
	num_colors = len(rgb_values)
	fig, ax = plt.subplots(1, num_colors, figsize=(num_colors*2, 2))

	for i, rgb in enumerate(rgb_values):
		ax[i].imshow([[rgb]], extent=[0, 1, 0, 1], aspect='auto')
		ax[i].axis('off')

	plt.show()

# Example usage
if __name__ == "__main__":
	rgb_colors = [
		[0, 0, 200],
		[0, 100, 200],
		[0, 200, 200],
		[100, 0, 0],
		[100, 0, 100],
		[0, 100, 0],
		[200, 100, 0],
		[200, 200, 0],
	]
	normalized_colors = normalize_rgb(rgb_colors)
	visualize_colors(normalized_colors)

