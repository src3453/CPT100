// fixed.hpp
// 固定小数点フォーマット定義 & 汎用テンプレート
//
// 選定仕様 (PS1風を意識しつつ本プロジェクト向けに最適化)
// ---------------------------------------------------------
// 1) ワールド位置     : Q20.12 (32bit, 符号付)  range ≒ [-524288, +524287.99976]
//    - 大きめのワールド想定。12bit 小数で 1/4096 精度 (~0.00024)
// 2) 法線ベクトル     : Q1.15  (16(内部32)bit, 符号付) range ≒ [-1.0, 0.99997]
//    - 単位長前提。乗算回数多いので 15bit fractional を確保。
// 3) UV座標           : Q10.6  (16(内部32)bit, 0以上推奨) range ≒ [-1024, 1023.984375]
//    - テクスチャサイズ 1024 未満前提。サブピクセル 1/64。
// 4) スクリーン座標   : Q24.8  (32bit, 符号付)  range ≒ 十分 (画面 384x288 用サブピクセル 1/256)
//    - ラスタライズ時のサブピクセル & 勾配計算で安定。
// 5) 深度(Z)          : Q16.8  (24(内部32)bit, 符号付)  range ≒ 適度 (クリップ後 0..1~数百を射影)
//
// ※ 24bit 実ストレージは扱いづらいので 32bit に保持しつつ Bits でマスク。省メモリ最適化は後段。
// ※ 後で必要なら Qx.y を個別再調整可。頂点/インデックスパック時の仕様書は docs/3d_memory.md で追記予定。
//
// 実装方針
// ---------------------------------------------------------
//  - テンプレート Fixed<Bits,Frac,StoreT=int32_t>
//  - 内部 raw_ (StoreT) に 2 の Frac 乗でスケール。
//  - 四則演算 / 比較 / 変換 (int,float,double) / saturate キャスト提供。
//  - 乗算/除算は 64bit 中間 (int64_t) で精度確保。
//  - Bits が 32 未満の場合は setRaw()/sanitize() でマスクしオーバーフロー挙動を規定。
//  - constexpr で scale, one など提供。
//  - 将来 SIMD / 固定行列最適化余地保持。
//
// 注意
// ---------------------------------------------------------
//  - 現段階では例外を投げない (ゼロ除算はデバッグ用アサートのみ)。
//  - パック/アンパック (頂点ストリーム) は後続タスクで別ユーティリティを追加。
//  - clamp 用に saturateAdd/sub 提供 (必要最低限)。
#pragma once

#include <cstdint>
#include <type_traits>
#include <limits>
#include <cmath>
#include <cassert>

// 汎用固定小数テンプレート
template<int Bits, int Frac, typename StoreT = int32_t>
class Fixed {
    static_assert(Bits > 0 && Bits <= (int)(sizeof(StoreT)*8), "Bits out of range");
    static_assert(Frac >= 0 && Frac < Bits, "Frac out of range");
public:
    using This = Fixed<Bits,Frac,StoreT>;
    using WideCalc = int64_t;

    // 定数
    static constexpr int   kBits   = Bits;
    static constexpr int   kFrac   = Frac;
    static constexpr StoreT kFracMask = (StoreT)((((uint64_t)1)<<Frac)-1);
    static constexpr StoreT kBitMask  = (Bits == (int)(sizeof(StoreT)*8)) ? (StoreT)(~StoreT(0))
                                        : (StoreT)((((uint64_t)1)<<Bits)-1);
    static constexpr StoreT kSignBit  = (StoreT)1 << (Bits-1);
    static constexpr StoreT kScale    = (StoreT)1 << Frac;

    static constexpr This fromRaw(StoreT raw, bool sanitize=true) {
        This v;
        v.raw_ = sanitize ? sanitizeBits(raw) : raw;
        return v;
    }
    static constexpr This fromInt(int v) {
        return fromRaw((StoreT)v * kScale);
    }
    static constexpr This fromFloat(float f) {
        float scaled = f * (float)kScale;
        WideCalc r = (WideCalc)std::llround((long double)scaled);
        return fromRaw((StoreT)r);
    }
    static constexpr This fromDouble(double d) {
        double scaled = d * (double)kScale;
        WideCalc r = (WideCalc)std::llround((long double)scaled);
        return fromRaw((StoreT)r);
    }

    // デフォルト 0
    constexpr Fixed(): raw_(0) {}
    // int
    constexpr Fixed(int v) : raw_((StoreT)v * kScale) { sanitize(); }
    // float/double
    constexpr Fixed(float v)  : raw_((StoreT)std::llround((long double)(v * kScale))) { sanitize(); }
    constexpr Fixed(double v) : raw_((StoreT)std::llround((long double)(v * kScale))) { sanitize(); }

    // 基本変換
    constexpr int toIntTrunc() const { return (int)(raw_ / kScale); }
    constexpr int toIntRound() const {
        StoreT add = (raw_ >= 0) ? (kScale>>1) : -(kScale>>1);
        return (int)((raw_ + add)/kScale);
    }
    constexpr float  toFloat()  const { return (float)raw_ / (float)kScale; }
    constexpr double toDouble() const { return (double)raw_ / (double)kScale; }

    // アクセス
    constexpr StoreT raw() const { return raw_; }

    // 演算
    constexpr This operator+(const This& o) const {
        return fromRaw(sanitizeBits(raw_ + o.raw_), false);
    }
    constexpr This operator-(const This& o) const {
        return fromRaw(sanitizeBits(raw_ - o.raw_), false);
    }
    constexpr This operator-() const {
        return fromRaw(sanitizeBits(-raw_), false);
    }
    constexpr This operator*(const This& o) const {
        // (a*b)>>Frac
        WideCalc r = (WideCalc)raw_ * (WideCalc)o.raw_;
        r = r >> Frac;
        return fromRaw((StoreT)r);
    }
    constexpr This operator/(const This& o) const {
        assert(o.raw_ != 0 && "divide by zero");
        WideCalc numerator = ((WideCalc)raw_ << Frac);
        WideCalc r = numerator / (WideCalc)o.raw_;
        return fromRaw((StoreT)r);
    }

    constexpr This& operator+=(const This& o) { raw_ = sanitizeBits(raw_ + o.raw_); return *this; }
    constexpr This& operator-=(const This& o) { raw_ = sanitizeBits(raw_ - o.raw_); return *this; }
    constexpr This& operator*=(const This& o) {
        WideCalc r = (WideCalc)raw_ * (WideCalc)o.raw_;
        r >>= Frac;
        raw_ = sanitizeBits((StoreT)r);
        return *this;
    }
    constexpr This& operator/=(const This& o) {
        assert(o.raw_ != 0 && "divide by zero");
        WideCalc numerator = ((WideCalc)raw_ << Frac);
        WideCalc r = numerator / (WideCalc)o.raw_;
        raw_ = sanitizeBits((StoreT)r);
        return *this;
    }

    // 比較
    constexpr bool operator==(const This& o) const { return raw_ == o.raw_; }
    constexpr bool operator!=(const This& o) const { return raw_ != o.raw_; }
    constexpr bool operator< (const This& o) const { return raw_ <  o.raw_; }
    constexpr bool operator<=(const This& o) const { return raw_ <= o.raw_; }
    constexpr bool operator> (const This& o) const { return raw_ >  o.raw_; }
    constexpr bool operator>=(const This& o) const { return raw_ >= o.raw_; }

    // 線形補間
    static constexpr This lerp(const This& a, const This& b, const This& t) {
        // a + (b-a)*t
        return a + (b - a) * t;
    }

    // 乗算後スケール自由 (Fracが異なる型への変換などは別途実装予定)
    template<int OBITS,int OFRAC, typename OStore>
    constexpr auto mulWide(const Fixed<OBITS,OFRAC,OStore>& o) const {
        using OutWide = int64_t;
        OutWide r = (OutWide)raw_ * (OutWide)o.raw();
        return r; // 呼び出し側でシフト調整
    }

    // Saturate 加算 (簡易)
    constexpr This saturateAdd(const This& o) const {
        WideCalc r = (WideCalc)raw_ + (WideCalc)o.raw_;
        if constexpr (Bits < (int)(sizeof(StoreT)*8)) {
            // 2の補数 saturate
            WideCalc maxv = (WideCalc)((StoreT)( (kBitMask>>1) ));
            WideCalc minv = -maxv - 1;
            if (r > maxv*kScale) r = maxv*kScale;
            if (r < minv*kScale) r = minv*kScale;
        } else {
            if (r > (WideCalc)std::numeric_limits<StoreT>::max()) r = std::numeric_limits<StoreT>::max();
            if (r < (WideCalc)std::numeric_limits<StoreT>::min()) r = std::numeric_limits<StoreT>::min();
        }
        return fromRaw((StoreT)r);
    }

    // 正規化（Bits < full の時のみマスク）
    constexpr void sanitize() {
        if constexpr (Bits < (int)(sizeof(StoreT)*8)) {
            // 2 の補数符号拡張を保つため一旦マスク後、符号ビットで拡張
            StoreT masked = raw_ & kBitMask;
            // 符号拡張
            if (masked & kSignBit) {
                StoreT extendMask = ~kBitMask;
                masked |= extendMask;
            }
            raw_ = masked;
        }
    }

private:
    StoreT raw_;

    static constexpr StoreT sanitizeBits(StoreT v) {
        if constexpr (Bits < (int)(sizeof(StoreT)*8)) {
            StoreT masked = v & kBitMask;
            if (masked & kSignBit) {
                StoreT extendMask = ~kBitMask;
                masked |= extendMask;
            }
            return masked;
        } else {
            return v;
        }
    }
};

// 代表型エイリアス
using fx_pos     = Fixed<32,12>; // 位置
using fx_normal  = Fixed<16,15>; // 法線 (内部 32bit 保管)
using fx_uv      = Fixed<16,6>;  // UV
using fx_screen  = Fixed<32,8>;  // スクリーン座標
using fx_depth   = Fixed<24,8>;  // 深度 (内部 32bit)

// 汎用ユーティリティ
template<class F>
inline F fx_from_float(float f) { return F::fromFloat(f); }

template<class F>
inline float fx_to_float(const F& v) { return v.toFloat(); }

// 簡易テスト/デバッグ (必要なら main から呼び出し)
// 後でユニットテスト導入時に移行
inline void fx_debug_example() {
    fx_pos a = fx_pos::fromFloat(1.25f);
    fx_pos b = fx_pos::fromFloat(-0.5f);
    fx_pos c = a + b;
    (void)c;
}
