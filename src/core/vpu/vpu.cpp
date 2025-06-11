// VPU: Vector Processing Unit
// for 3D graphics rendering

#include "buffer.cpp"
#include "triangle.cpp"

class VPU
{
private:
  CPT_Screen &screen;
  /* data */
public:
  VPU(CPT_Screen &screen) : screen(screen)
  {
    // Initialize the VPU with the screen object
    // Any other initialization code can go here
  }

  ~VPU()
  {
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
