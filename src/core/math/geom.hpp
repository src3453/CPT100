// geom.hpp
// 固定小数点ベクトル / 行列表現 & 基本変換/射影ユーティリティ
#pragma once
#include <cstdint>
#include <array>
#include <cmath>
#include "fixed.hpp"

// Fallback (spec.hpp が未インクルードでも利用できるように)
// 他で既に定義されていれば再定義しない
#ifndef CPT_SCREEN_WIDTH
#define CPT_SCREEN_WIDTH 384
#endif
#ifndef CPT_SCREEN_HEIGHT
#define CPT_SCREEN_HEIGHT 288
#endif

// ベクトル型 (汎用テンプレート)
// ScaleFracT: 乗算後のシフト調整などで利用したい場合に拡張可 (今回は未使用)
template<class FX>
struct Vec2T {
    FX x,y;
    constexpr Vec2T():x(0),y(0){}
    constexpr Vec2T(FX x, FX y):x(x),y(y){}
    static Vec2T fromFloat(float fx, float fy){ return Vec2T(FX::fromFloat(fx), FX::fromFloat(fy)); }
    float xf() const { return x.toFloat(); }
    float yf() const { return y.toFloat(); }

    Vec2T operator+(const Vec2T& o) const { return Vec2T(x+o.x,y+o.y); }
    Vec2T operator-(const Vec2T& o) const { return Vec2T(x-o.x,y-o.y); }
    Vec2T operator-() const { return Vec2T(-x,-y); }
    Vec2T operator*(FX s) const { return Vec2T(x*s,y*s); }

    FX dot(const Vec2T& o) const { return x*o.x + y*o.y; }
};

template<class FX>
struct Vec3T {
    FX x,y,z;
    constexpr Vec3T():x(0),y(0),z(0){}
    constexpr Vec3T(FX x, FX y, FX z):x(x),y(y),z(z){}
    static Vec3T fromFloat(float fx, float fy, float fz){ return Vec3T(FX::fromFloat(fx), FX::fromFloat(fy), FX::fromFloat(fz)); }
    float xf() const { return x.toFloat(); }
    float yf() const { return y.toFloat(); }
    float zf() const { return z.toFloat(); }

    Vec3T operator+(const Vec3T& o) const { return Vec3T(x+o.x,y+o.y,z+o.z); }
    Vec3T operator-(const Vec3T& o) const { return Vec3T(x-o.x,y-o.y,z-o.z); }
    Vec3T operator-() const { return Vec3T(-x,-y,-z); }
    Vec3T operator*(FX s) const { return Vec3T(x*s,y*s,z*s); }

    FX dot(const Vec3T& o) const { return x*o.x + y*o.y + z*o.z; }

    Vec3T cross(const Vec3T& o) const {
        // 中間は浮動小数経由簡易実装 (後で固定小数最適化可)
        float ax=xf(), ay=yf(), az=zf();
        float bx=o.xf(), by=o.yf(), bz=o.zf();
        return Vec3T::fromFloat(
            ay*bz - az*by,
            az*bx - ax*bz,
            ax*by - ay*bx
        );
    }

    // 正規化 (法線向け: fx_normal 想定) - 暫定で float 正規化
    Vec3T normalized() const {
        float ax=xf(), ay=yf(), az=zf();
        float len = std::sqrt(ax*ax+ay*ay+az*az);
        if (len <= 0.0000001f) return *this;
        float inv = 1.0f/len;
        return Vec3T::fromFloat(ax*inv, ay*inv, az*inv);
    }
};

// 代表用途エイリアス
using Vec3Pos = Vec3T<fx_pos>;
using Vec3Nrm = Vec3T<fx_normal>;
using Vec2UV  = Vec2T<fx_uv>;
using Vec3Scr = Vec3T<fx_screen>; // x,y: screen, z: depth 用にも使用可

// 4x4 行列 (固定小数点: 要素は fx_pos を基準)
struct Mat4 {
    // 行優先(row-major)
    fx_pos m[4][4];

    static Mat4 identity() {
        Mat4 R;
        for(int r=0;r<4;r++) for(int c=0;c<4;c++) R.m[r][c] = fx_pos::fromInt(r==c?1:0);
        return R;
    }

    static Mat4 translate(fx_pos tx, fx_pos ty, fx_pos tz) {
        Mat4 R = identity();
        R.m[0][3] = tx;
        R.m[1][3] = ty;
        R.m[2][3] = tz;
        return R;
    }

    static Mat4 scale(fx_pos sx, fx_pos sy, fx_pos sz) {
        Mat4 R;
        for(int r=0;r<4;r++) for(int c=0;c<4;c++) R.m[r][c]=fx_pos::fromInt(0);
        R.m[0][0]=sx; R.m[1][1]=sy; R.m[2][2]=sz; R.m[3][3]=fx_pos::fromInt(1);
        return R;
    }

    static Mat4 rotY(float radf) {
        // rad を float で生成 (後で固定 trig 導入可)
        float c = std::cos(radf);
        float s = std::sin(radf);
        Mat4 R = identity();
        R.m[0][0] = fx_pos::fromFloat(c);
        R.m[0][2] = fx_pos::fromFloat(s);
        R.m[2][0] = fx_pos::fromFloat(-s);
        R.m[2][2] = fx_pos::fromFloat(c);
        return R;
    }

    static Mat4 perspective(float fovY_deg, float aspect, float zNear, float zFar) {
        // 右手系想定, NDC: z 0..1 (将来変更可)
        float f = 1.0f/std::tan(fovY_deg * 3.1415926535f/180.0f * 0.5f);
        Mat4 R;
        for(int r=0;r<4;r++) for(int c=0;c<4;c++) R.m[r][c]=fx_pos::fromInt(0);
        R.m[0][0] = fx_pos::fromFloat(f/aspect);
        R.m[1][1] = fx_pos::fromFloat(f);
        R.m[2][2] = fx_pos::fromFloat((zFar)/(zFar - zNear)); // 0..1
        R.m[2][3] = fx_pos::fromFloat((-zNear*zFar)/(zFar - zNear));
        R.m[3][2] = fx_pos::fromFloat(1.0f);
        return R;
    }

    // LookAt (簡易右手系) eye, target, up
    static Mat4 lookAt(const Vec3Pos& eye, const Vec3Pos& target, const Vec3Pos& up) {
        // float で計算 -> 固定へ
        float ex=eye.xf(), ey=eye.yf(), ez=eye.zf();
        float tx=target.xf(), ty=target.yf(), tz=target.zf();
        float ux=up.xf(), uy=up.yf(), uz=up.zf();
        float zx=ex-tx, zy=ey-ty, zz=ez-tz;
        float zlen = std::sqrt(zx*zx+zy*zy+zz*zz);
        if (zlen>0){ zx/=zlen; zy/=zlen; zz/=zlen; }
        float xx = uy*zz - uz*zy;
        float xy = uz*zx - ux*zz;
        float xz = ux*zy - uy*zx;
        float xlen = std::sqrt(xx*xx+xy*xy+xz*xz);
        if (xlen>0){ xx/=xlen; xy/=xlen; xz/=xlen; }
        float yx = zy*xz - zz*xy;
        float yy = zz*xx - zx*xz;
        float yz = zx*xy - zy*xx;

        Mat4 R;
        R.m[0][0]=fx_pos::fromFloat(xx); R.m[0][1]=fx_pos::fromFloat(yx); R.m[0][2]=fx_pos::fromFloat(zx); R.m[0][3]=fx_pos::fromFloat(- (xx*ex + yx*ey + zx*ez));
        R.m[1][0]=fx_pos::fromFloat(xy); R.m[1][1]=fx_pos::fromFloat(yy); R.m[1][2]=fx_pos::fromFloat(zy); R.m[1][3]=fx_pos::fromFloat(- (xy*ex + yy*ey + zy*ez));
        R.m[2][0]=fx_pos::fromFloat(xz); R.m[2][1]=fx_pos::fromFloat(yz); R.m[2][2]=fx_pos::fromFloat(zz); R.m[2][3]=fx_pos::fromFloat(- (xz*ex + yz*ey + zz*ez));
        R.m[3][0]=fx_pos::fromInt(0);  R.m[3][1]=fx_pos::fromInt(0);  R.m[3][2]=fx_pos::fromInt(0);  R.m[3][3]=fx_pos::fromInt(1);
        return R;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 R;
        for(int r=0;r<4;r++){
            for(int c=0;c<4;c++){
                fx_pos acc = fx_pos::fromInt(0);
                for(int k=0;k<4;k++){
                    acc = acc + m[r][k]*o.m[k][c];
                }
                R.m[r][c]=acc;
            }
        }
        return R;
    }

    // 頂点位置変換 (w 透視除算は別)
    Vec3Pos transformPoint(const Vec3Pos& v) const {
        fx_pos X = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3];
        fx_pos Y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3];
        fx_pos Z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3];
        fx_pos W = m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3];
        // 透視除算 (W!=0) float 経由
        float wf = W.toFloat();
        if (std::fabs(wf) > 1e-6f) {
            float inv = 1.0f / wf;
            return Vec3Pos::fromFloat(X.toFloat()*inv, Y.toFloat()*inv, Z.toFloat()*inv);
        }
        return Vec3Pos::fromFloat(X.toFloat(), Y.toFloat(), Z.toFloat());
    }

    // 法線変換 (回転部分のみ / 正規化任意)
    Vec3Nrm transformNormal(const Vec3Nrm& n) const {
        // 異フォーマット (fx_pos と fx_normal) 直接積の演算子未実装のため
        // float へ展開してから行列回転を適用し再量子化
        float nx = n.x.toFloat();
        float ny = n.y.toFloat();
        float nz = n.z.toFloat();
        float Xf = m[0][0].toFloat()*nx + m[0][1].toFloat()*ny + m[0][2].toFloat()*nz;
        float Yf = m[1][0].toFloat()*nx + m[1][1].toFloat()*ny + m[1][2].toFloat()*nz;
        float Zf = m[2][0].toFloat()*nx + m[2][1].toFloat()*ny + m[2][2].toFloat()*nz;
        Vec3Nrm out{
            fx_normal::fromFloat(Xf),
            fx_normal::fromFloat(Yf),
            fx_normal::fromFloat(Zf)
        };
        return out.normalized();
    }
};

// スクリーン射影補助
// NDC (-1..1) をスクリーン px (サブピクセル) に変換
inline Vec3Scr ndcToScreen(const Vec3Pos& ndc) {
    float nx = ndc.xf();
    float ny = ndc.yf();
    float nz = ndc.zf(); // 0..1 を想定
    float sx = (nx * 0.5f + 0.5f) * (float)CPT_SCREEN_WIDTH;
    float sy = (-ny * 0.5f + 0.5f) * (float)CPT_SCREEN_HEIGHT; // 上方向反転
    // 深度: 0..1 を fx_depth へ
    return Vec3Scr{
        fx_screen::fromFloat(sx),
        fx_screen::fromFloat(sy),
        fx_screen::fromFloat(nz) // z は後段で fx_depth に変換可
    };
}

// 深度変換
inline fx_depth toDepth(const Vec3Scr& scr) {
    return fx_depth::fromFloat(scr.z.toFloat()); // 必要なら非線形->線形補正
}

// 将来: 三角形エッジ勾配計算 / 面法線 / 平面方程式 等の固定小数最適化を追加予定