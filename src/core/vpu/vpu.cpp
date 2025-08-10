 // VPU: Vector Processing Unit
 // for 3D graphics rendering
 
 #include <cstdint>
 #include <vector>
 #include <cstring>
 #include <string>
 #include <stdexcept>
 
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
 
 // メッシュ / テクスチャ ダンプフォーマット (docs/3d_memory.md v0 に準拠) 簡易定義
 struct C3DM_FileHeader {
   uint32_t magic;            // 'C','3','D','M' 0x4D443343
   uint16_t version;
   uint16_t materialCount;
   uint32_t meshCount;
   uint32_t materialTableOffset;
   uint32_t meshTableOffset;
   uint32_t reserved[4];
 };
 
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
     if (fh->magic != 0x4D443343) return fail("invalid file magic (expect C3DM)");
     if (fh->meshCount == 0) return fail("no meshes");
     if (fh->meshTableOffset + fh->meshCount * sizeof(uint32_t) > size) return fail("meshTable overflow");
 
     // 1つ目のメッシュのみ
     const uint32_t* meshOffsets = reinterpret_cast<const uint32_t*>(data + fh->meshTableOffset);
     uint32_t mo = meshOffsets[0];
     if (mo + sizeof(C3DM_MeshChunkHeader) > size) return fail("mesh header out of range");
     const C3DM_MeshChunkHeader* mh = reinterpret_cast<const C3DM_MeshChunkHeader*>(data + mo);
     if (mh->magic != 0x4D455348) return fail("mesh magic mismatch (MESH)");
 
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
   void debugDrawLoadedMeshWire(uint8_t color = 255)
   {
     if (!meshLoaded) return;
     // 簡易スケール: Q20.12 → float へ
     auto q20_12_to_float = [](int32_t v)->float{ return (float)v / 4096.0f; };
     // 画面中央
     float cx = CPT_SCREEN_WIDTH * 0.5f;
     float cy = CPT_SCREEN_HEIGHT * 0.5f;
     for (size_t i=0;i+2<loadedIndices.size(); i+=3) {
       const C3DM_VertexPacked &a = loadedVertices[loadedIndices[i]];
       const C3DM_VertexPacked &b = loadedVertices[loadedIndices[i+1]];
       const C3DM_VertexPacked &c = loadedVertices[loadedIndices[i+2]];
       int ax = (int)(cx + q20_12_to_float(a.px)*32.0f);
       int ay = (int)(cy - q20_12_to_float(a.py)*32.0f);
       int bx = (int)(cx + q20_12_to_float(b.px)*32.0f);
       int by = (int)(cy - q20_12_to_float(b.py)*32.0f);
       int cxp= (int)(cx + q20_12_to_float(c.px)*32.0f);
       int cyp= (int)(cy - q20_12_to_float(c.py)*32.0f);
       // 線分描画 (Bresenham でなく既存 line を利用したい場合はスクリーン API 経由)
       // ここでは単純な DDA 風
       auto drawLine = [&](int x0,int y0,int x1,int y1){
         int dx = x1 - x0;
         int dy = y1 - y0;
         int steps = std::max(std::abs(dx), std::abs(dy));
         if (steps<=0) return;
         float fx = (float)x0;
         float fy = (float)y0;
         float incx = (float)dx / steps;
         float incy = (float)dy / steps;
         for (int s=0;s<=steps;s++){
           // screen.pix は後方で定義 (インクルード順の都合でリンケージは main.cpp 依存)
           screen.pix((int)fx,(int)fy,color);
           fx += incx; fy += incy;
         }
       };
       drawLine(ax,ay,bx,by);
       drawLine(bx,by,cxp,cyp);
       drawLine(cxp,cyp,ax,ay);
     }
   }
 
   // Hardware T&L: Transform and Lighting

  int dither(int x, int y, float value)
  {
    // dithering helper function
    int ditherMatrix[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};

    if (fmod(value, 1) < ditherMatrix[y % 4][x % 4] / 16.0)
    {
      return ceil(value); // 白
    }
    else
    {
      return floor(value); // 黒
    }
  }

  void TexturedTriangle(vpuTexBuffer tex, vpuTriangle triangle)
  {
    // 三角形の頂点を y座標でソート（上から下へ）
    if (triangle.y1 > triangle.y2)
    {
      std::swap(triangle.x1, triangle.x2);
      std::swap(triangle.y1, triangle.y2);
      std::swap(triangle.u1, triangle.u2);
      std::swap(triangle.v1, triangle.v2);
      std::swap(triangle.z1, triangle.z2);
    }
    if (triangle.y1 > triangle.y3)
    {
      std::swap(triangle.x1, triangle.x3);
      std::swap(triangle.y1, triangle.y3);
      std::swap(triangle.u1, triangle.u3);
      std::swap(triangle.v1, triangle.v3);
      std::swap(triangle.z1, triangle.z3);
    }
    if (triangle.y2 > triangle.y3)
    {
      std::swap(triangle.x2, triangle.x3);
      std::swap(triangle.y2, triangle.y3);
      std::swap(triangle.u2, triangle.u3);
      std::swap(triangle.v2, triangle.v3);
      std::swap(triangle.z2, triangle.z3);
    }

    // 三角形を平坦化して、スキャンラインでレンダリングできるようにする
    int totalHeight = triangle.y3 - triangle.y1;
    if (totalHeight == 0)
      return; // 三角形の高さが0の場合は何も描画しない

    // 上半分の三角形を描画
    for (int y = triangle.y1; y <= triangle.y2; y++)
    {
      int segmentHeight = triangle.y2 - triangle.y1;
      if (segmentHeight == 0)
        continue;

      float alpha = (float)(y - triangle.y1) / totalHeight;
      float beta = (float)(y - triangle.y1) / segmentHeight;

      int x1 = triangle.x1 + (triangle.x3 - triangle.x1) * alpha;
      int x2 = triangle.x1 + (triangle.x2 - triangle.x1) * beta;

      // テクスチャ座標とZ値の補間
      float u1 = triangle.u1 + (triangle.u3 - triangle.u1) * alpha;
      float v1 = triangle.v1 + (triangle.v3 - triangle.v1) * alpha;
      float z1 = triangle.z1 + (triangle.z3 - triangle.z1) * alpha;

      float u2 = triangle.u1 + (triangle.u2 - triangle.u1) * beta;
      float v2 = triangle.v1 + (triangle.v2 - triangle.v1) * beta;
      float z2 = triangle.z1 + (triangle.z2 - triangle.z1) * beta;

      if (x1 > x2)
      {
        std::swap(x1, x2);
        std::swap(u1, u2);
        std::swap(v1, v2);
        std::swap(z1, z2);
      }

      // この水平線のピクセルを描画
      for (int x = x1; x <= x2; x++)
      {
        float phi = (x2 == x1) ? 1.0 : (float)(x - x1) / (float)(x2 - x1);
        float u = u1 + (u2 - u1) * phi;
        float v = v1 + (v2 - v1) * phi;
        float z = z1 + (z2 - z1) * phi;

        // 画面外チェック
        if (x < 0 || y < 0 || x >= CPT_SCREEN_WIDTH || y >= CPT_SCREEN_HEIGHT)
          continue;

        // Z値チェック（深度バッファがある場合）
        // if (z < depthBuffer[y*width + x])
        //   continue;

        // テクスチャ座標の正規化とマッピング
        int tx = (int)(u * tex.width) % tex.width;
        int ty = (int)(v * tex.height) % tex.height;

        // テクスチャフォーマットに応じたカラー取得
        unsigned char color;
        if (tex.format == TEX_PALETTE)
        {
          // パレットモード
          color = tex.texBuffer[ty * tex.width + tx];
        }
        else
        {
          // RGBモードの場合は平均色を計算して単色にする
          int idx = (ty * tex.width + tx) * 3;
          int r = tex.texBuffer[idx];
          int g = tex.texBuffer[idx + 1];
          int b = tex.texBuffer[idx + 2];
          color = (r + g + b) / 3; // グレースケール変換
        }

        // ピクセルの描画（実際の描画システムに合わせて実装）
        // pix(x, y, color);
      }
    }

    // 下半分の三角形を描画
    for (int y = triangle.y2; y <= triangle.y3; y++)
    {
      int segmentHeight = triangle.y3 - triangle.y2;
      if (segmentHeight == 0)
        continue;

      float alpha = (float)(y - triangle.y1) / totalHeight;
      float beta = (float)(y - triangle.y2) / segmentHeight;

      int x1 = triangle.x1 + (triangle.x3 - triangle.x1) * alpha;
      int x2 = triangle.x2 + (triangle.x3 - triangle.x2) * beta;

      // テクスチャ座標とZ値の補間
      float u1 = triangle.u1 + (triangle.u3 - triangle.u1) * alpha;
      float v1 = triangle.v1 + (triangle.v3 - triangle.v1) * alpha;
      float z1 = triangle.z1 + (triangle.z3 - triangle.z1) * alpha;

      float u2 = triangle.u2 + (triangle.u3 - triangle.u2) * beta;
      float v2 = triangle.v2 + (triangle.v3 - triangle.v2) * beta;
      float z2 = triangle.z2 + (triangle.z3 - triangle.z2) * beta;

      if (x1 > x2)
      {
        std::swap(x1, x2);
        std::swap(u1, u2);
        std::swap(v1, v2);
        std::swap(z1, z2);
      }

      // この水平線のピクセルを描画
      for (int x = x1; x <= x2; x++)
      {
        float phi = (x2 == x1) ? 1.0 : (float)(x - x1) / (float)(x2 - x1);
        float u = u1 + (u2 - u1) * phi;
        float v = v1 + (v2 - v1) * phi;
        float z = z1 + (z2 - z1) * phi;

        // 画面外チェック
        if (x < 0 || y < 0 || x >= 384 || y >= 288)
          continue;

        // Z値チェック（深度バッファがある場合）
        // if (z < depthBuffer[y*width + x])
        //   continue;

        // テクスチャ座標の正規化とマッピング
        int tx = (int)(u * tex.width) % tex.width;
        int ty = (int)(v * tex.height) % tex.height;

        // テクスチャフォーマットに応じたカラー取得
        unsigned char color;
        if (tex.format == TEX_PALETTE)
        {
          // パレットモード
          color = tex.texBuffer[ty * tex.width + tx];
        }
        else
        {
          // RGBモードの場合は平均色を計算して単色にする
          int idx = (ty * tex.width + tx) * 3;
          int r = tex.texBuffer[idx];
          int g = tex.texBuffer[idx + 1];
          int b = tex.texBuffer[idx + 2];
          color = (r + g + b) / 3; // グレースケール変換
        }

        // ピクセルの描画（実際の描画システムに合わせて実装）
        screen.pix(x, y, color);
      }
    }
  }
};
