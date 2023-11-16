
// ここにコードを追加してください
int i = 0;
for (int y = 0; y < 384 ; y++) {
    for (int x = 0; x < 384 ; x++) {
        scr.pix(x,y,i%256);
        i++;
    }
}

font.print("Hello, world!",0,0,255);