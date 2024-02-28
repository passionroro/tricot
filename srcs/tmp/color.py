import tkinter as tk

def convert_rgb_to_hex(rgb):
    """
    Convert RGB values to hexadecimal color code.
    """
    r, g, b = map(lambda x: round(float(x)), rgb.split(','))
    return f'#{r:02x}{g:02x}{b:02x}'

def display_color(rgb):
    """
    Display color in a window using Tkinter.
    """
    hex_color = convert_rgb_to_hex(rgb)

    root = tk.Tk()
    root.title("RGB Color Converter")

    frame = tk.Frame(root, width=200, height=200, bg=hex_color)
    frame.pack(fill=tk.BOTH, expand=True)

    root.mainloop()

def read_rgb_from_file(file_path):
    """
    Read RGB values from a file and display corresponding colors.
    """
    with open(file_path, 'r') as file:
        for line in file:
            rgb = line.strip()
            display_color(rgb)

# Example usage
read_rgb_from_file("4.txt")

