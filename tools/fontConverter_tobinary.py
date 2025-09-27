def read_cpp_header(file_path):
    """
    Read the C++ header file and extract the font data.
    """
    font_data = []
    with open(file_path, "r") as f:
        for line in f:
            line = line.strip().replace("},", "}")
            if line.startswith("{") and line.endswith("}"):
                # Extract hex values from the line
                hex_values = line[1:-1].split(", ")
                font_data.append(bytes(int(h, 16) for h in hex_values))
    return font_data

def write_f12_file(font_data, output_path):
    """
    Write the font data to a binary *.f12 file.
    """
    with open(output_path, "wb") as f:
        for char_data in font_data:
            f.write(char_data)

if __name__ == "__main__":
    # Example usage
    input_file = r"C:\Users\y2k34\remote\CPT100\src\core\res\font8x12.hpp"  # Replace with actual file path
    output_file = r"C:\Users\y2k34\remote\CPT100\font8x12.f12"  # Replace with desired output path

    try:
        font_data = read_cpp_header(input_file)
        write_f12_file(font_data, output_file)
        print(f"Successfully converted {input_file} to {output_file}")
    except Exception as e:
        print(f"Error: {e}")