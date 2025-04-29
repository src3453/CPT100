#include <stdexcept>

enum TexFormat {
  TEX_PALETTE,
  TEX_RGB 
};

class vpuTexBuffer
{
private:
  /* data */
public:
  unsigned int addr; // Address of the texture buffer in memory
  unsigned int width; // Width of the texture
  unsigned int height; // Height of the texture
  TexFormat format; // Format of the texture (palette or RGB)
  unsigned char *texBuffer; // Pointer to the texture buffer
  vpuTexBuffer::vpuTexBuffer(unsigned int width, unsigned int height, TexFormat format)
  {
    // Allocate memory for the texture buffer based on the format
    if (format == TEX_PALETTE) {
      // Allocate memory for palette texture
      texBuffer = new unsigned char[width * height];
    } else if (format == TEX_RGB) {
      // Allocate memory for RGB texture
      texBuffer = new unsigned char[width * height * 3]; // 3 bytes per pixel for RGB
    }
    this->width = width;
    this->height = height;
    this->format = format;

  }

  void markAsTexBuffer(unsigned int addr) {
    if (addr < 0x000000 || addr > RAM_SIZE) {
      throw std::out_of_range("Address out of range. Must be between 0x000000 and system RAM size.");
    } else {
      this->addr = addr; //0x000000 ~ 0xFFFFFF
    }
  }

  unsigned char* fetchTexBuffer() {
    texBuffer = ram_peek2array(ram, addr, width * height * (format == TEX_PALETTE ? 1 : 3)).data();
    return texBuffer;
  }

  void writeTexBuffer(unsigned char *texture) {
    int n = sizeof(texture) / sizeof(texture[0]);
    std::vector<unsigned char> vectorTex(texture, texture + n);
    ram_poke2array(ram, addr, vectorTex);
  }
 
  ~vpuTexBuffer()
  {
    // Free the allocated memory
    delete[] texBuffer;
  }

};


