 // VPU: Vector Processing Unit
 // for 3D graphics rendering and Textured Triangle Rasterization
 // with OpenGL acceleration
 // Fixed pipeline, no shaders, Hardware T&L style

 /*
 VRAM Layout:
 0x20000 - 0x7FFFF: for VPU (3D engine registers, Mesh and Texture Buffers)
 inside this range:
  0x20000 - 0x20FFF: VPU Registers
  0x21000 - 0x5FFFF: Mesh Buffer (can render 10752 vertices theoretically)
  0x60000 - 0x7FFFF: Texture Buffer (can store 131072 bytes of texture data, 256x512 in 256 colors palette texture)
 future expansion:
  - Banked VRAM access for larger mesh/texture storage
  - Multiple texture pages
  - Compressed data formats (DXT1 etc)
 */
 
 #include <cstdint>
 #include <vector>
 #include <cstring>
 #include <string>
 #include <stdexcept>
 #include <SDL_opengl.h>
 #include <SDL_opengl_glext.h>

 // Define function pointers for FBO
 PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
 PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
 PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
 PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
 PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
 PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
 PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
 PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
 PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;

 
 // 依存 (既存ローカル実装)
 #include "buffer.cpp"
 #include "triangle.cpp"
 
 // 前方宣言 (他翻訳単位で定義)
 //#include "../disp.cpp"
 
 // 画面サイズマクロ (spec.hpp が既にどこかで include されている前提 / 未定義ならデフォルト)
 #ifndef Byte
 #define Byte unsigned char
 #endif

 #ifndef CPT_SCREEN_WIDTH
 #define CPT_SCREEN_WIDTH 384
 #endif
 #ifndef CPT_SCREEN_HEIGHT
 #define CPT_SCREEN_HEIGHT 288
 #endif

 #define VPU_REG_BASE 0x20000
 #define VPU_MESH_BASE 0x21000
 #define VPU_TEXTURE_BASE 0x60000
 
 // メッシュ / テクスチャ ダンプフォーマット (docs/3d_memory.md v0 に準拠) 簡易定義
 // C3DM: CPT 3D Model Format (*.c3d / *.c3dm)

 // ファイルヘッダ (36 bytes)
 struct C3DM_FileHeader {
   uint32_t magic;            // 'C','3','D','M' 0x4D443343
   uint16_t version;
   uint16_t materialCount;
   uint32_t meshCount;
   uint32_t materialTableOffset;
   uint32_t meshTableOffset;
   uint32_t reserved[4];
 };
 
 // メッシュチャンクヘッダ (20 bytes)
 struct C3DM_MeshChunkHeader {
   uint32_t magic;           // 'MESH'
   uint16_t version;
   uint16_t flags;           // bit0: index32
   uint32_t vertexCount;
   uint32_t indexCount;      // 3 の倍数
   uint32_t materialId;
 };
 
 // 頂点パック (24 bytes)
 #pragma pack(push,1)
 struct C3DM_VertexPacked {
   int32_t px,py,pz;       // Q20.12
   int16_t nx,ny,nz;       // Q1.15
   uint16_t u,v;           // Q10.6
   uint16_t pad;           // reserved
 };
 #pragma pack(pop)
 
 class VPU
 {
 private:
   CPT_Screen &screen;
 
   // ロード結果格納 (最初のメッシュのみ簡易格納)
   std::vector<C3DM_VertexPacked> loadedVertices;
   std::vector<uint32_t>          loadedIndices;  // 16/32 どちらも 32 に詰め替え
   bool meshLoaded = false;

   // OpenGL Resources
   GLuint fbo = 0;
   GLuint renderTex = 0;
   GLuint depthRb = 0;
   bool glReady = false;

   void initGL() {
       if (glReady) return;
       
       // Load extensions
       glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
       glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
       glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
       glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
       glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
       glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorage");
       glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
       glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
       glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");
       
       if (!glGenFramebuffers) {
           // Try EXT
           glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT");
           glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
           glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
           glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
           glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
           glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
           glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
           glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
           glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
       }
       
       if (!glGenFramebuffers) {
           printf("Failed to load FBO functions\n");
           return;
       }

       glGenFramebuffers(1, &fbo);
       glBindFramebuffer(GL_FRAMEBUFFER, fbo);

       glGenTextures(1, &renderTex);
       glBindTexture(GL_TEXTURE_2D, renderTex);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
       glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

       glGenRenderbuffers(1, &depthRb);
       glBindRenderbuffer(GL_RENDERBUFFER, depthRb);
       glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT);
       glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRb);

       glBindFramebuffer(GL_FRAMEBUFFER, 0);
       glReady = true;
   }
 
 public:
   VPU(CPT_Screen &screen) : screen(screen)
   {
   }
 
   ~VPU() = default;
 
   // ダンプからメッシュをロード (単一メッシュ簡易対応)
   bool loadMeshFromMemory(const uint8_t* data, size_t size, std::string* errMsg = nullptr)
   {
    auto fail = [&](const std::string& m){
      if(errMsg) *errMsg = m;
      return false;
    };
    if (size < sizeof(C3DM_FileHeader)) return fail("size too small for FileHeader");
    const C3DM_FileHeader* fh = reinterpret_cast<const C3DM_FileHeader*>(data);
    if (fh->magic != 0x4D443343) {
      printf("wrong magic: %08X\n", fh->magic);
      return fail("invalid file magic (expect C3DM)");
    }
    if (fh->meshCount == 0) return fail("no meshes");
    if (fh->meshTableOffset + fh->meshCount * sizeof(uint32_t) > size) return fail("meshTable overflow");
 
     // 1つ目のメッシュのみ
     const uint32_t* meshOffsets = reinterpret_cast<const uint32_t*>(data + fh->meshTableOffset);
     uint32_t mo = meshOffsets[0];
     if (mo + sizeof(C3DM_MeshChunkHeader) > size) return fail("mesh header out of range");
     const C3DM_MeshChunkHeader* mh = reinterpret_cast<const C3DM_MeshChunkHeader*>(data + mo);
     if (mh->magic != 0x4853454D) {
      printf("wrong mesh magic: %08X\n", mh->magic);
      return fail("mesh magic mismatch (MESH)");
    }
 
     size_t vertBytes = mh->vertexCount * sizeof(C3DM_VertexPacked);
     size_t idxBytes  = mh->indexCount * ((mh->flags & 0x1)? 4 : 2);
     size_t payloadStart = mo + sizeof(C3DM_MeshChunkHeader);
     size_t need = payloadStart + vertBytes + idxBytes;
     if (need > size) return fail("mesh payload out of range");
 
     const C3DM_VertexPacked* vp = reinterpret_cast<const C3DM_VertexPacked*>(data + payloadStart);
     loadedVertices.assign(vp, vp + mh->vertexCount);
 
     const uint8_t* idxBase = data + payloadStart + vertBytes;
     loadedIndices.clear();
     loadedIndices.reserve(mh->indexCount);
     if (mh->flags & 0x1) {
       // 32bit
       const uint32_t* idx32 = reinterpret_cast<const uint32_t*>(idxBase);
       for (uint32_t i=0;i<mh->indexCount;i++) loadedIndices.push_back(idx32[i]);
     } else {
       // 16bit
       const uint16_t* idx16 = reinterpret_cast<const uint16_t*>(idxBase);
       for (uint32_t i=0;i<mh->indexCount;i++) loadedIndices.push_back(idx16[i]);
     }
     meshLoaded = true;
     return true;
   }
 
   bool isMeshLoaded() const { return meshLoaded; }
 
   // デバッグ用: ロード済みメッシュを簡易描画 (直投影 + XY 切出し)
   // 今は px/py をスクリーン中央へオフセットして粗描画
  // 統合された描画関数
  // mode: 0=Wireframe, 1=Solid, 2=Textured
  void renderFrame(int mode, float scale = 1.0f)
  {
    if (!meshLoaded) return;
    initGL();
    
    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    int WIDTH = CPT_SCREEN_WIDTH;
    if (screenMode == 3) {
        WIDTH = CPT_SCREEN_WIDTH / 2;
    }
    
    glViewport(0, 0, WIDTH, CPT_SCREEN_HEIGHT);
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, CPT_SCREEN_WIDTH, CPT_SCREEN_HEIGHT, 0, -1000, 1000);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GLuint textureID = 0;
    if (mode == 2) {
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        // VRAMからテクスチャをロード (256x512 8bit Paletteと仮定)
        auto texData = vram_peek2array(vram, VPU_TEXTURE_BASE, 131072);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 512, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texData.data());
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    if (mode == 0) { // Wireframe
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_POLYGON_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
    } else { // Solid or Textured
        glEnable(GL_DEPTH_TEST);
        glColor3f(1.0f, 1.0f, 1.0f);
        if (mode == 1) {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glShadeModel(GL_SMOOTH);
        } else {
            glDisable(GL_LIGHTING);
        }
        glBegin(GL_TRIANGLES);
    }
    
    auto q20_12_to_float = [](int32_t v)->float{ return (float)v / 4096.0f; };
    float cx = CPT_SCREEN_WIDTH * 0.5f;
    float cy = CPT_SCREEN_HEIGHT * 0.5f;
    
    for (size_t i=0;i+2<loadedIndices.size(); i+=3) {
       const C3DM_VertexPacked &a = loadedVertices[loadedIndices[i]];
       const C3DM_VertexPacked &b = loadedVertices[loadedIndices[i+1]];
       const C3DM_VertexPacked &c = loadedVertices[loadedIndices[i+2]];
       const float Factor = 32.0f * scale; 
       
       float ax = cx + q20_12_to_float(a.px)*Factor;
       float ay = cy - q20_12_to_float(a.py)*Factor;
       float az = q20_12_to_float(a.pz)*Factor;
       
       float bx = cx + q20_12_to_float(b.px)*Factor;
       float by = cy - q20_12_to_float(b.py)*Factor;
       float bz = q20_12_to_float(b.pz)*Factor;
       
       float cxp= cx + q20_12_to_float(c.px)*Factor;
       float cyp= cy - q20_12_to_float(c.py)*Factor;
       float czp= q20_12_to_float(c.pz)*Factor;

       if (mode == 0) {
           glVertex2f(ax, ay); glVertex2f(bx, by);
           glVertex2f(bx, by); glVertex2f(cxp, cyp);
           glVertex2f(cxp, cyp); glVertex2f(ax, ay);
       } else {
           if (mode == 2) {
               // Q10.6 -> Pixel Coords -> UV (0..1)
               float u1 = (float)a.u / 64.0f / 256.0f;
               float v1 = (float)a.v / 64.0f / 512.0f;
               float u2 = (float)b.u / 64.0f / 256.0f;
               float v2 = (float)b.v / 64.0f / 512.0f;
               float u3 = (float)c.u / 64.0f / 256.0f;
               float v3 = (float)c.v / 64.0f / 512.0f;
               
               glTexCoord2f(u1, v1);
               glVertex3f(ax, ay, az);
               glTexCoord2f(u2, v2);
               glVertex3f(bx, by, bz);
               glTexCoord2f(u3, v3);
               glVertex3f(cxp, cyp, czp);
           } else {
               glNormal3f(
                (float)a.nx / 32767.0f,
                (float)a.ny / 32767.0f,
                (float)a.nz / 32767.0f
               );
               glVertex3f(ax, ay, az);
               glVertex3f(bx, by, bz);
               glVertex3f(cxp, cyp, czp);
           }
       }
    }
    glEnd();
    
    if (mode == 2) {
        glDeleteTextures(1, &textureID);
        glDisable(GL_TEXTURE_2D);
    }
    if (mode == 1) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
    }
    if (mode == 0) {
        glDisable(GL_BLEND);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    std::vector<uint8_t> pixels(WIDTH * CPT_SCREEN_HEIGHT * 4);
    glReadPixels(0, 0, WIDTH, CPT_SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    
    for (int y = 0; y < CPT_SCREEN_HEIGHT; y++) {
         for (int x = 0; x < WIDTH; x++) {
             int glY = CPT_SCREEN_HEIGHT - 1 - y;
             int idx = (glY * WIDTH + x) * 4;
             if (pixels[idx+3] > 1) { 
                 screen.pix(x, y, fromRGBDithered(x,y,pixels[idx], pixels[idx+1], pixels[idx+2]));
             }
         }
     }
  }

  void updateState(){
    std::string err;
    bool success = loadMeshFromMemory(vram_peek2array(vram, VPU_MESH_BASE, RAM_SIZE - VPU_MESH_BASE).data(), RAM_SIZE - VPU_MESH_BASE, &err);
    if (!success) {
      printf("VPU Mesh Load Error: %s\n", err.c_str());
      return;
    }
  }

  void renderDebug(float scale = 1.0f)
  {
    renderFrame(0, scale);
  }

  void renderDebug2(float scale = 1.0f)
  {
    renderFrame(1, scale);
  }

  void render(){
    renderFrame(2, 1.0f);
  }
};
