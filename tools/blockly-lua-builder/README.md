# CPT200 Blockly Lua Builder

CPT200 の Lua API を使って、Blockly で BOOT/LOOP を組み立て、Lua コードを生成するための簡易ビルダーです。

## 収録APIブロック
- Display: `screen(mode)`, `cls(color)`
- Drawing: `pix(x,y,color)`, `rect(x,y,w,h,color)`, `line(xs,ys,xe,ye,color)`, `rgb(r,g,b)`, `rgbd(x,y,r,g,b)`
- Text: `print(text,x,y,color)`
- Sprites: `spr(index, enabled, x, y, rotation)`
- Input: `mouse() → local mx,my,mb`, `key(keycode)`
- Time: `time()`

必要に応じて他のAPI（peek/vpeek/poke/vpoke、配列版、音関連、カーソル制御等）も追加可能です。

## 使い方
1. VS Code でこのワークスペースを開いた状態で、`tools/blockly-lua-builder/index.html` をブラウザで開きます。
   - Windowsならエクスプローラーでダブルクリック、または簡易HTTPサーバー：
   ```powershell
   cd tools/blockly-lua-builder
   python -m http.server 8000
   # ブラウザで http://localhost:8000/index.html を開く
   ```
2. 左のツールボックスからブロックを配置し、`Program` の BOOT/LOOP 入力に処理を繋ぎます。
3. 右側に生成された Lua コードを確認し、`Luaをエクスポート` ボタンで `blockly.lua` をダウンロードします。
4. 生成コードを `src/lua/main.lua` に貼り付けて保存し、プロジェクトをビルドします。
   - 既存の `LOOP()` / `BOOT()` 構造を置き換えたい場合は、バックアップを取ってから差し替えてください。

## 既知の注意点
- `mouse()` の戻り値は 3 値（x,y,ボタン）想定です。本ビルダーではローカル変数に代入する補助ブロックのみ提供しています。
- API 名はプロジェクトの Lua 側で小文字（例: `print`, `pix`, `cls`, `screen`）として使われている前提です。
- より高度なブロック（タイルパターン、FFT、DMA等）は段階的に追加予定です。

## 拡張の提案
- 追加APIのブロック化（peek/poke系、カーソル/テキストプレーン、音入力/FFT、スプライトロードなど）。
- 生成Luaのヘッダー自動変換（`lua/*.lua.hpp`）パイプライン統合。
- 既存サンプル（`src/lua/main.lua`）のパレット化（よく使うパターンをブロックテンプレート化）。
