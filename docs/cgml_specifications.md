# CGML (CPT200 Graphic Markup Language) 仕様書
CGML (CPT200 Graphic Markup Language) は、CPT200 デバイスで使用されるHTMLベースのグラフィックマークアップ言語で、レトロな表現をするために機能が制限されています。この仕様書では、CGML の構造、要素、および使用方法について説明します。
# 1. ファイル構造
CGML ファイルは、以下のセクションで構成されます:
- CGML 宣言 (`<cgml>` ~ `</cgml>` タグ)
  - メタデータ (`<meta>` タグ)
  - lua スクリプト (`<script>` タグ)
  - スタイルシート (`<style>` タグ)
  - コンテンツ (`<body>` タグ)
各セクションは、特定のタグで囲まれています。
# 2. 要素
## 2.1 CGML 宣言
CGML ファイルは、必ず `<cgml>` タグで始まり、`</cgml>` タグで終わります。これにより、CGML ファイルであることが示されます。
```xml
<cgml>
  <meta>
    <title>Example CGML Document</title>
    <author>John Doe</author>
    <date>2024-06-01</date>
    <description>This is an example of a CGML document.</description>
    <screenmode>graphical</screenmode> <!-- 'text', 'graphical'(デフォルト), 'graphical-high'(未実装) -->
  </meta>
</cgml>
```
## 2.2 メタデータ
メタデータは `<meta>` タグ内に配置され、ドキュメントのタイトル、著者、日付、説明、および画面モードを含みます。
- `<title>`: ドキュメントのタイトルを指定します。
- `<author>`: 著者の名前を指定します。
- `<date>`: ドキュメントの作成日を指定します。
- `<description>`: ドキュメントの簡単な説明を提供します。
- `<screenmode>`: 画面モードを指定します。`text`(48x24文字), `graphical`(384x288@256色), `graphical-high`(未実装, 768x576@16色) のいずれかを使用します。 デフォルトは `graphical` です。`text`モードではグラフィック要素は無視されます。
- `<charset>`: 文字エンコーディングを指定します。 `cp437` (デフォルト), `unicode`(UTF-16BE, BMPのみ) が使用可能です。
## 2.3 スクリプト
CGML ファイル内で Lua スクリプトを使用する場合、`<script>` タグ内にコードを配置します。
```xml
<script>
    -- Lua スクリプトコード
    function onLoad()
        CGML.console.log("CGML document loaded.")
    end
</script>
```
Lua スクリプトには、CPT200 の Lua API と、CGML の API を使用して、ドキュメントの動的な動作を制御できます。
## 2.4 スタイルシート
スタイルシートは `<style>` タグ内に配置され、CGML ドキュメントの外観を定義します。CSS のサブセットが使用されます。
```xml
<style>
    body {
        background-color: #000000; /* RGBは自動的にインデックスカラーに変換されます */
        color: index(255); /* インデックスカラー (16色/256色) */
    }
    h1 {
        color: #FF0000;
    }
</style>
```
## 2.5 コンテンツ
ドキュメントの主要なコンテンツは `<body>` タグ内に配置されます。テキスト、画像、リンクなどの要素を含むことができます。
```xml
<body>
    <h1>Welcome to CGML</h1>
    <p>This is a sample CGML document.</p>
    <button onclick="CGML.console.log('Button clicked!')">Click Me</button>
    <img src="image.png" alt="Sample Image"/>
</body>
```
# 3. レンダリングパイプライン
CGML のレンダリングエンジンは、Lua スクリプトで実装されており、CPT200 Lua API のグラフィックシステムを使用してドキュメントを表示します。
CGML ドキュメントのレンダリングは、以下の手順で行われます:
1. ファイルの読み込みと解析 (CGML タグの検証, DOM ツリーの構築)
2. メタデータの適用 (画面モード, 文字セットの設定)
3. スタイルシートの適用 (CSS ルールの適用)
4. コンテンツのレンダリング (テキスト, 画像, ボタンなどの描画)
5. スクリプトの実行 (Lua スクリプトの初期化とイベントハンドリング)