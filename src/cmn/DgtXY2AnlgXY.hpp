/**
 *  @file   DbgXY2AnlgXY.hpp
 *  @brief  デジタル十字ボタン入力をアナログ値に変換
 *  @author Masashi KITAMURA
 *  @date   2006
 *  @note
 *      フリーソース
 */
#ifndef DGTXY2ANLGXY_HPP
#define DGTXY2ANLGXY_HPP

#pragma once
#include <cassert>
#include <cmath>

/** on/offのみのカーソルボタン入力から、擬似アナログ値を生成.
 *  アナログ値の扱いが場合によって違うのでテンプレート引数で設定.
 *  @param ANALOG_T     x,yの値の型. デフォルトfloat
 *  @param MAX_VAL      アナログ値として返す最大値. デフォルト 1(float用).
 *  @param HIS_USE_NUM  ボタン履歴数. 5～16くらい.
 */
template<typename ANALOG_T=float, unsigned MAX_VAL=1, unsigned HIS_USE_NUM=10>
class DgtXY2AnlgXY {
public:
    DgtXY2AnlgXY();
    ~DgtXY2AnlgXY(){}
    ANALOG_T    analogX() const { return analogX_; }
    ANALOG_T    analogY() const { return analogY_; }
    void        set(int dx, int dy);

private:
    static ANALOG_T mySqrt(ANALOG_T val);

private:
    ANALOG_T    analogX_;
    ANALOG_T    analogY_;
    struct xy_t { signed char x_, y_; };
    xy_t        his_[ HIS_USE_NUM + 1 ];        ///< 入力の履歴
};



/** コンストラクタ
 */
template<typename ANALOG_T, unsigned MAX_VAL, unsigned HIS_USE_NUM>
DgtXY2AnlgXY<ANALOG_T,MAX_VAL,HIS_USE_NUM>::DgtXY2AnlgXY()
{
    assert( MAX_VAL     > 0 );
    assert( HIS_USE_NUM > 1 );
    for (unsigned i = 0; i < HIS_USE_NUM; ++i) {
        his_[i].x_ = 0;
        his_[i].y_ = 0;
    }
}



/** 平方根
 */
template<typename ANALOG_T, unsigned MAX_VAL, unsigned HIS_USE_NUM>
inline ANALOG_T
DgtXY2AnlgXY<ANALOG_T,MAX_VAL,HIS_USE_NUM>::mySqrt(ANALOG_T val) {
    return ANALOG_T( std::sqrtf(val) );
}


/** 今回のxy入力
 */
template<typename ANALOG_T, unsigned MAX_VAL, unsigned HIS_USE_NUM>
void DgtXY2AnlgXY<ANALOG_T,MAX_VAL,HIS_USE_NUM>::set(int dx, int dy)
{
    assert( -1 <= dx && dx <= 1 );
    assert( -1 <= dy && dy <= 1 );
    his_[HIS_USE_NUM].x_ = (signed char)dx;
    his_[HIS_USE_NUM].y_ = (signed char)dy;
    ANALOG_T x = 0;
    ANALOG_T y = 0;
    for (unsigned i = 0; i < HIS_USE_NUM; ++i) {
        his_[i] = his_[i+1];
        x += his_[i].x_;
        y += his_[i].y_;
    }
    // ALALOG_Tが整数の時に、精度をマシにするためにKを用意.
    enum { K = (HIS_USE_NUM <= 64) ? 128 / HIS_USE_NUM : 1 };
    x *= K;
    y *= K;
    ANALOG_T    l = mySqrt(x*x+y*y);
    ANALOG_T    r = l;
    if (r >= K*HIS_USE_NUM)
        r = K*HIS_USE_NUM;
    if (l > ANALOG_T(0.0001f) && (dx | dy)) {   // dx,dyもチェックして、余分な慣性を無くす
        x = x * r * MAX_VAL / (l * K*HIS_USE_NUM);
        y = y * r * MAX_VAL / (l * K*HIS_USE_NUM);
    } else {
        x = 0;
        y = 0;
    }
    analogX_ = x;
    analogY_ = y;
}


#endif
