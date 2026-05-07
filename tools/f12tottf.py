#!/usr/bin/env python3
"""
8x12 CP437 Bitmap Font (.f12) to TrueType Font (TTF) Converter

.f12形式: 256文字 × 12バイト/文字 = 3072バイト
各文字は12バイト（各バイトが8ビット幅の1行を表す）
"""

import sys
import os
from pathlib import Path
from math import ceil, floor

try:
    from fontTools.pens.ttGlyphPen import TTGlyphPen
    from fontTools.ttLib import TTFont
    from fontTools.fontBuilder import FontBuilder
except ImportError:
    print("エラー: fontToolsがインストールされていません。")
    print("インストール方法:")
    print("  pip install fonttools")
    sys.exit(1)


def read_f12_font(filepath):
    """
    .f12ファイルを読み込んでビットマップデータを返す
    
    Returns:
        list: 256個の文字のビットマップデータ（各文字は12x8のビット配列）
    """
    with open(filepath, 'rb') as f:
        data = f.read()
    
    if len(data) != 3072:
        raise ValueError(f"ファイルサイズが不正です: {len(data)} bytes (期待値: 3072 bytes)")
    
    glyphs = []
    for char_idx in range(256):
        # 各文字は12バイト
        char_data = []
        for row in range(12):
            byte_val = data[char_idx * 12 + row]
            char_data.append(byte_val)
        glyphs.append(char_data)
    
    return glyphs


def create_ttf_font(glyphs, output_path, font_name="F12Font"):
    """
    ビットマップデータからTTFフォントを生成
    
    Args:
        glyphs: 256個の文字のビットマップデータ
        output_path: 出力TTFファイルのパス
        font_name: フォント名
    """
    from fontTools.pens.ttGlyphPen import TTGlyphPen
    
    # フォントの基本設定
    char_width = 8
    char_height = 12
    units_per_em = 1000
    scale = units_per_em / char_height
    
    # グリフの作成
    glyph_order = [".notdef"] + [f"char{i:03d}" for i in range(256)]
    
    # 文字マッピング (cmap)
    cmap = {i: f"char{i:03d}" for i in range(256)}
    
    # グリフデータの作成
    glyphs_dict = {}
    metrics = {}
    
    # .notdef グリフ（空のグリフ）
    pen = TTGlyphPen(None)
    glyphs_dict[".notdef"] = pen.glyph()
    metrics[".notdef"] = (int(char_width * scale), 0)  # (width, lsb)
    
    # 各文字のグリフを作成
    for char_idx, glyph_data in enumerate(glyphs[:256]):
        glyph_name = f"char{char_idx:03d}"
        width = int(char_width * scale)
        
        pen = TTGlyphPen(None)
        
        # LSB (Left Side Bearing) を計算するための最小X座標
        min_x = None
        
        # ビットマップからパスを作成
        for y, byte_val in enumerate(glyph_data):
            for x in range(8):
                if byte_val & (0x80 >> x):
                    # ピクセルを四角形として描画
                    # Y座標を反転（フォントは下から上）
                    # 隙間防止のためにceil, floorを使用
                    px = floor(x * scale)
                    py = floor((char_height - 1 - y) * scale)
                    pw = ceil(scale)
                    ph = ceil(scale)
                    
                    # 最小X座標を更新
                    if min_x is None or px < min_x:
                        min_x = px
                    
                    # 四角形のパスを描画
                    pen.moveTo((px, py))
                    pen.lineTo((px + pw, py))
                    pen.lineTo((px + pw, py + ph))
                    pen.lineTo((px, py + ph))
                    pen.closePath()
        
        # 空のグリフ（スペースなど）の場合はLSBを0にする
        if min_x is None:
            min_x = 0
            
        glyphs_dict[glyph_name] = pen.glyph()
        metrics[glyph_name] = (width, min_x)  # (width, lsb)
    
    # FontBuilderを使用してフォントを構築
    fb = FontBuilder(units_per_em, isTTF=True)
    fb.setupGlyphOrder(glyph_order)
    fb.setupCharacterMap(cmap)
    
    # フォント情報を設定
    fb.setupPost(isFixedPitch=True)
    
    # headテーブルを先に設定
    fb.setupHead(
        unitsPerEm=units_per_em,
        created=0,
        lowestRecPPEM=8
    )
    
    # hheaテーブルとhmtxテーブルを設定
    fb.setupHorizontalHeader(
        ascent=int(char_height * scale * 0.8),
        descent=-int(char_height * scale * 0.2)
    )
    fb.setupHorizontalMetrics(metrics)
    
    # OS/2テーブルを設定（hmtxの後）
    fb.setupOS2(
        sTypoAscender=int(char_height * scale * 0.8),
        sTypoDescender=-int(char_height * scale * 0.2),
        sTypoLineGap=0,
        usWinAscent=int(char_height * scale),
        usWinDescent=0
    )
    
    # nameテーブルを設定
    fb.setupNameTable({
        "familyName": font_name,
        "styleName": "Regular",
        "uniqueFontIdentifier": f"{font_name}-Regular",
        "fullName": f"{font_name} Regular",
        "psName": font_name,
        "version": "Version 1.0"
    })
    
    # glyfテーブルを設定（TrueType用）
    fb.setupGlyf(glyphs_dict)
    
    # フォントを保存
    font = fb.font
    font.save(output_path)
    print(f"フォントを生成しました: {output_path}")


def main():
    if len(sys.argv) < 2:
        print("使い方: python f12tottf.py <input.f12> [output.ttf]")
        print("例: python f12tottf.py font8x12.f12 font8x12.ttf")
        sys.exit(1)
    
    input_path = sys.argv[1]
    
    if len(sys.argv) >= 3:
        output_path = sys.argv[2]
    else:
        # 拡張子を.ttfに変更
        output_path = Path(input_path).with_suffix('.ttf')
    
    if not os.path.exists(input_path):
        print(f"エラー: ファイルが見つかりません: {input_path}")
        sys.exit(1)
    
    print(f"入力ファイル: {input_path}")
    print(f"出力ファイル: {output_path}")
    print()
    
    # フォントを読み込む
    print("ビットマップフォントを読み込んでいます...")
    glyphs = read_f12_font(input_path)
    print(f"  {len(glyphs)} 文字を読み込みました")
    
    # TTFフォントを生成
    print("TTFフォントを生成しています...")
    font_name = Path(input_path).stem
    create_ttf_font(glyphs, str(output_path), font_name)
    
    print()
    print("完了!")


if __name__ == "__main__":
    main()
