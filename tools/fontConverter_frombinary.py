import struct

def read_f12_file(file_path):
    """
    Read a *.f12 file and parse its binary data.
    Each character is represented by 12 bytes, and there are 256 characters in total.
    """
    with open(file_path, "rb") as f:
        data = f.read()
        if len(data) != 12 * 256:
            raise ValueError("Invalid file size. Expected 3072 bytes.")
        
        # Split data into 256 characters, each with 12 bytes
        characters = [data[i * 12:(i + 1) * 12] for i in range(256)]
        return characters

def convert_to_cpp_header(characters, output_path):
    """
    Convert the binary font data into a C++ header file format.
    """
    with open(output_path, "w") as f:
        f.write("#ifndef FONT8X12_HPP\n")
        f.write("#define FONT8X12_HPP\n\n")
        f.write("const unsigned char font8x12[256][12] = {\n")
        
        for i, char in enumerate(characters):
            hex_values = ", ".join(f"0x{byte:02X}" for byte in char)
            f.write(f"    {{ {hex_values} }}")
            if i < len(characters) - 1:
                f.write(",\n")
            else:
                f.write("\n")
        
        f.write("};\n\n")
        f.write("#endif // FONT8X12_HPP\n")

if __name__ == "__main__":
    # Example usage
    input_file = "C:\\Users\\y2k34\\Downloads\\cpt200.f12"  # Replace with actual file path
    output_file = "C:\\Users\\y2k34\\remote\\CPT100\\src\\core\\res\\font8x12.hpp_test"  # Replace with actual output path

    try:
        font_data = read_f12_file(input_file)
        convert_to_cpp_header(font_data, output_file)
        print(f"Successfully converted font data to {output_file}")
    except Exception as e:
        print(f"Error: {e}")