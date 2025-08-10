# 3D メモリレイアウト仕様 (暫定 v0)

本ドキュメントは 3D パイプライン (固定小数点 / メモリダンプ読込) のためのバイナリ／論理レイアウト仕様暫定版。  
将来の最適化 / 圧縮 / SIMD 対応を見越して「読み込みやすさ優先」で設計。

---

## 1. 固定小数点フォーマット採用一覧

| 用途 | 型エイリアス | 形式 | ビット長(符号含) | 小数部 | 代表値例 | 備考 |
|------|--------------|------|------------------|--------|----------|------|
| ワールド位置 xyz | fx_pos | Q20.12 | 32bit | 12 | 1.0 => 4096 | 広いマップ対応 |
| 法線 xyz | fx_normal | Q1.15 | 16bit(内部32) | 15 | 1.0 => 32768 | 単位ベクトル前提 |
| UV (u,v) | fx_uv | Q10.6 | 16bit | 6 | 1.0 => 64 | 最大~1024 テクスチャ想定 |
| スクリーン座標 xy | fx_screen | Q24.8 | 32bit | 8 | 1.0 => 256 | サブピクセル (1/256) |
| 深度 z | fx_depth | Q16.8 (24bit運用/格納32) | 24/32 | 8 | 1.0 => 256 | Zバッファ/比較用 |

※ `fx_normal` は演算過程で 32bit に昇格するが格納は 16bit を想定したパックを別途用意可能。

---

## 2. 頂点ストリーム バイナリ構造 (暫定)

### 2.1 各頂点レコード (可変ヘッダ無し・固定長)

```
struct VertexPacked {
  int32  px;   // fx_pos Q20.12
  int32  py;   // "
  int32  pz;   // "
  int16  nx;   // fx_normal Q1.15
  int16  ny;
  int16  nz;
  uint16 u;    // fx_uv Q10.6 (符号付が必要なら int16 に変更)
  uint16 v;
  // 追加予定: color(頂点色) / tangent / bitangent / weight 等
};
```

1 頂点: 3*4 + 3*2 + 2*2 = 12 + 6 + 4 = 22 bytes  
アライメント簡易化のため **2 bytes パディング** を付け 24 bytes/vertex にすることを推奨:

```
uint16 _pad; // 0 or future use
```

→ 24 bytes * N 頂点

### 2.2 インデックス

```
uint16 indices[ numTriangles * 3 ];
```

最大頂点数が 65535 を超える場合は 32bit 化 (`uint32`) モードを別途ヘッダフラグで指示。

### 2.3 メッシュチャンクヘッダ (1 メッシュ単位)

```
struct MeshChunkHeader {
  uint32 magic;        // 'MESH' (0x4D455348)
  uint16 version;      // 0x0001
  uint16 flags;        // bit0: index32 使用, bit1: vertex_color あり, etc.
  uint32 vertexCount;
  uint32 indexCount;   // 3 の倍数
  uint32 materialId;   // マテリアル参照 ID (後述)
};
```

続いて `VertexPacked[vertexCount]` → インデックス配列 → (オプション: 頂点カラー, 追加チャンネル)

---

## 3. マテリアル / テクスチャ

### 3.1 テクスチャヘッダ (パレット or RGB)

```
struct TextureHeader {
  uint32 magic;      // 'TEX0'
  uint16 version;    // 0x0001
  uint16 format;     // 0:PAL8  1:RGB24 (将来: 2:RGB565 3:RGBA4444 ...)
  uint16 width;
  uint16 height;
  uint16 clutCount;  // パレットエントリ数 (PAL8 の場合 256 / 16 など)
  uint16 flags;      // ミップ有無, ラップモードなど
};
```

続き:

- format==PAL8: `uint8 indices[width*height]` + `uint8 clut[clutCount*3]`
- format==RGB24: `uint8 data[width*height*3]`

### 3.2 マテリアル (簡易)

```
struct Material {
  uint32 magic;       // 'MAT0'
  uint16 version;     // 0x0001
  uint16 flags;       // bit0: textured, bit1: gouraud, bit2: alpha
  uint32 texOffset;   // ファイル先頭からの TextureHeader へのオフセット (0 で無し)
  uint8  baseColor[3];// 未使用時は一定シェーディング色
  uint8  pad;
};
```

将来: CLUT ID, ライティング係数, その他パラメータ等追加。

---

## 4. ファイル全体構成例

```
[ FileHeader ]
[ MaterialTable ... ]
[ TextureChunk ... ] * M
[ MeshChunkHeader + Vertex + Index ... ] * K
```

### 4.1 FileHeader

```
struct FileHeader {
  uint32 magic;        // 'C3DM'
  uint16 version;      // 0x0001
  uint16 materialCount;
  uint32 meshCount;
  uint32 materialTableOffset;
  uint32 meshTableOffset;  // MeshChunk オフセット配列
  uint32 reserved[4];
};
```

`meshTableOffset` から `uint32 meshOffsets[meshCount]` を列挙し各オフセットへ。

---

## 5. 固定小数点 パック / アンパック仕様

| 型 | 内部 float f からのパック | 逆変換 |
|----|---------------------------|--------|
| fx_pos (Q20.12) | raw = (int32)round(f * 4096.0) | f = raw / 4096.0 |
| fx_normal (Q1.15) | raw = (int16)round(clamp(f,-1,0.99997)*32768) | f = raw / 32768.0 |
| fx_uv (Q10.6) | raw = (int16)round(f * 64.0) | f = raw / 64.0 |
| fx_screen (Q24.8) | raw = (int32)round(f * 256.0) | f = raw / 256.0 |
| fx_depth (Q16.8) | raw = (int32)round(f * 256.0) | f = raw / 256.0 |

法線は正規化してからパック。UV は基本非負だがタイル用に符号あり運用可。

---

## 6. Python 変換ツール (予定 `tools/convert3d.py`)

### 6.1 主機能
- OBJ ロード (位置 / 法線 / UV)
- 三角形化 (OBJ は既に三角の場合多い)
- 重複頂点マージ (頂点キー: (pos,normal,uv) 単純ハッシュ)
- 固定小数点へ変換
- バイナリ出力 (上記 FileHeader 方式)
- 追加: PNG 読み込み → パレット生成 (median cut / k-means 簡易) → パレット + インデックス格納

### 6.2 予定 CLI
```
python tools/convert3d.py \
  --input model.obj \
  --output model.c3dm \
  --texture tex.png \
  --palette 256 \
  --scale 1.0 \
  --gouraud on
```

### 6.3 フロー擬似コード
```
load_obj()
build_vertex_map()
pack_vertex() -> VertexPacked
build_index()
load_texture()
quantize_palette()
emit_file()
```

---

## 7. VPU 側ロード API (想定)

C++:
```
bool VPU::loadMeshFromMemory(const uint8_t* data, size_t size);
bool VPU::loadTextureFromMemory(const uint8_t* data, size_t size);
```

- ヘッダ検証
- エンドianness: リトルエンディアン固定
- 頂点 / インデックスを内部 CPU フレンドリ構造 or そのままストリームリングバッファへ
- 後段: DMA 風インタフェース (ram へ直接配置)

---

## 8. 深度 / ラスタライズ 方針 (概要)

1. モデル→ワールド→ビュー→射影 (固定行列)
2. NDC → スクリーン (ndcToScreen)
3. 三角形セットアップ:
   - エッジ方程式 (固定 Q24.8 ベース)
   - 1/z, u/z, v/z 透視補正用勾配 (float → 固定に再量子化)
4. Z バッファ: fx_depth (Q16.8) 配列
5. シェーディング:
   - フラット: 面法線 → 簡易拡散 (L·N)
   - Gouraud: 頂点カラー / 法線補間 (float 中間→固定)
6. テクスチャ:
   - 透視補正: u' = (u_over_z / inv_z), v' 同様
   - 最近傍: (uint16)→mod width/height
   - パレット: CLUT 参照 (既存 8bit→RGB24 変換を再利用)

---

## 9. 将来拡張メモ
- ストリップ化 / インデックス圧縮
- 可逆/非可逆頂点量子化 (より小さい bit packing)
- mipmap チャンク
- skeletal animation (weight + bone index)
- tangent space (tangent, bitangent)
- occlusion / lightmap UV チャンネル追加

---

## 10. サンプル: 1 頂点パック例

```
pos = (1.25, -0.5, 2.0)
px = round(1.25 * 4096)   = 5120
py = round(-0.5 * 4096)   = -2048
pz = round(2.0 * 4096)    = 8192

normal = (0,1,0)
ny = 32768 (0x8000) ではなく 1.0 -> 32768 だが int16 上限 32767 のため saturate → 32767

uv = (3.0, 1.5)
u = 3.0 * 64 = 192
v = 1.5 * 64 = 96
```

---

## 11. 注意点
- 現行 `geom.hpp` の `transformNormal` は float 経由。後日 `fx_pos * fx_normal` の混在乗算対応追加で完全固定化。
- endian 違いプラットフォーム対応は優先度低 (WASM / x86 メイン)。
- インデックス 32bit 対応時は `flags & 0x0001` をセット。

---

## 12. 今後のタスク関連 (対応マッピング)

| ドキュメント節 | 対応タスク |
|----------------|-----------|
| 1~5 | メモリレイアウト策定 |
| 6 | Python 変換ツール |
| 7 | メモリダンプ読込 API |
| 8 | 投影 / ラスタ / Z / シェーディング設計 |
| 10 | デバッグ / テストパターン |
| 11 | 最適化ロードマップ |

---

(End of spec v0)