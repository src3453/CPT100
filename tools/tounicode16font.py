import cv2
import numpy as np
import sys

# converts a JIS X 0208 bitmap font image to a Unicode 16 c++ formatted font file

def read_img_font(image_path):
    img = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise FileNotFoundError(f"Image file not found: {image_path}")
    img = cv2.threshold(img, 128, 1, cv2.THRESH_BINARY_INV)[1]
    return img

shift_jis = []
jisx0208 = []
unicode = []
with open("JIS0208.TXT", "r") as f:
    for line in f:
        if line[0] == "#":
            pass
        else:
            sjis, jisx, unic, _ = line.strip().split("\t")
            shift_jis.append(int(sjis,16))
            jisx0208.append( int(jisx,16))
            unicode.append(  int(unic,16))

def JISX0208_to_unicode16(n):
    try:
        idx = jisx0208.index(n)
        return unicode[idx]
    except ValueError:
        return 0x0000
    

def image_to_bytes(img, cell_width, cell_height, cols, rows, row_max=94, col_max=94):
    font_bytes = []
    for row in range(rows):
        for col in range(cols):
            cell = img[row*cell_height:(row+1)*cell_height, col*cell_width:(col+1)*cell_width]
            if sum(cell.flatten()) != 0:
                byte_row = []
                for y in range(cell_height):
                    byte = 0
                    for x in range(cell_width):
                        if cell[y, x]:
                            byte |= (1 << (cell_width - 1 - x))
                    byte_row.append(byte)
                codepoint = JISX0208_to_unicode16(row*256+col+0x2121) # JIS X 0208 starts at 0x2121
                font_bytes.extend([codepoint,byte_row]) # bitmap (12 bytes)
    return font_bytes

if __name__ == "__main__":

    img = read_img_font(sys.argv[1])
    font_bytes = image_to_bytes(img, 8, 12, 94, 94)

    # Write to C++ header file with key-value pairs for each character
    with open(sys.argv[2], "w") as f:
        f.write("// Generated font data\n")
        f.write("// Each character: 12 bytes bitmap (8x12)\n\n")
        f.write("#include <map>\n")
        f.write("#include <array>\n\n")
        f.write("const std::map<uint16_t, std::array<unsigned char, 12>> unicode16_font = {\n")
        for i in range(0, len(font_bytes), 2):
            codepoint = font_bytes[i]
            bitmap = font_bytes[i+1]
            f.write(f"\t{{0x{codepoint:04X}, "+"{"+",".join([f"0x{b:02X}" for b in bitmap])+"}},\n")
        f.write("};\n")

    print(f"Converted {len(font_bytes)} characters to {sys.argv[2]}")

