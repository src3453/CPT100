import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox
import re

class FontMaker:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("8x12 Font Maker")
        # 256文字×12行のビットマップ保存用
        self.char_data = [[0]*12 for _ in range(256)]
        self.char_index = 32  # 最初はスペース
        # セルボタン群
        self.cells = [[None]*8 for _ in range(12)]
        # ドラッグ塗りつぶし用フラグ
        self.paint_value = None
        self._build_ui()
        self._load_char()

    def _build_ui(self):
        frame = tk.Frame(self.root)
        frame.pack(padx=10, pady=10)
        for y in range(12):
            for x in range(8):
                b = tk.Button(frame, width=2, height=1)
                b.grid(row=y, column=x)
                self.cells[y][x] = b
                # クリック押下でペイント開始（塗り or 消去）
                b.bind("<ButtonPress-1>", lambda e, xx=x, yy=y: self._start_paint(xx, yy))
                # ドラッグ中のペイント継続
                b.bind("<B1-Motion>",     lambda e, xx=x, yy=y: self._paint(xx, yy))
                # ボタンリリースでペイント終了
                b.bind("<ButtonRelease-1>", lambda e: setattr(self, 'paint_value', None))
        ctrl = tk.Frame(self.root)
        ctrl.pack(pady=5)
        tk.Button(ctrl, text="Prev", command=self._prev_char).pack(side="left")
        tk.Button(ctrl, text="Next", command=self._next_char).pack(side="left")
        tk.Button(ctrl, text="Export", command=self._export).pack(side="left")
        tk.Button(ctrl, text="Import", command=self._import).pack(side="left")
        self.lbl = tk.Label(self.root, text="")
        self.lbl.pack()

    def _toggle(self, x, y):
        row = self.char_data[self.char_index][y]
        mask = 1 << (7-x)
        if row & mask:
            row &= ~mask
            self.cells[y][x].configure(bg="white")
        else:
            row |= mask
            self.cells[y][x].configure(bg="black")
        self.char_data[self.char_index][y] = row

    def _start_paint(self, x, y):
        # 初回クリック時のセル状態を反転させるモードで開始
        row = self.char_data[self.char_index][y]
        mask = 1 << (7-x)
        self.paint_value = not bool(row & mask)
        self._apply_paint(x, y)

    def _paint(self, x, y):
        if self.paint_value is None:
            return
        self._apply_paint(x, y)

    def _apply_paint(self, x, y):
        mask = 1 << (7-x)
        if self.paint_value:
            self.char_data[self.char_index][y] |= mask
            self.cells[y][x].configure(bg="black")
        else:
            self.char_data[self.char_index][y] &= ~mask
            self.cells[y][x].configure(bg="white")

    def _load_char(self):
        # 文字コードと文字自体を表示
        self.lbl.config(text=f"Code: 0x{self.char_index:02x} ('{chr(self.char_index)}')")
        for y in range(12):
            row = self.char_data[self.char_index][y]
            for x in range(8):
                color = "black" if (row & (1<<(7-x))) else "white"
                self.cells[y][x].configure(bg=color)

    def _save_current(self):
        # 変更はリアルタイムに保存しているので特になし
        pass

    def _prev_char(self):
        self._save_current()
        if self.char_index>0:
            self.char_index -=1
            self._load_char()

    def _next_char(self):
        self._save_current()
        if self.char_index<255:
            self.char_index +=1
            self._load_char()

    def _export(self):
        path = filedialog.asksaveasfilename(defaultextension=".h",
             filetypes=[("C Header","*.h")])
        if not path: return
        with open(path, "w") as f:
            f.write("unsigned char font[256][12] = {\n")
            for ci in range(256):
                f.write("  { ")
                for y in range(12):
                    b = self.char_data[ci][y]
                    f.write(f"0x{b:02X}")
                    if y<11: f.write(", ")
                f.write(" },\n")
            f.write("};\n")
        tk.messagebox.showinfo("Export", f"Saved to {path}")

    def _import(self):
        path = filedialog.askopenfilename(
            filetypes=[("C Source/Header","*.c;*.h"),("All files","*.*")])
        if not path: return
        text = open(path, "r", encoding="utf-8", errors="ignore").read()
        vals = re.findall(r'0x([0-9A-Fa-f]{2})', text)
        if len(vals) < 256*12:
            messagebox.showerror("Import", "フォントデータが不足しています")
            return
        for idx, hx in enumerate(vals[:256*12]):
            ci, y = divmod(idx, 12)
            self.char_data[ci][y] = int(hx, 16)
        messagebox.showinfo("Import", f"{path} から読み込みました")
        self._load_char()

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    FontMaker().run()
