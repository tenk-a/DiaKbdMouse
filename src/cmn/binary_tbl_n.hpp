#ifndef BINARY_TBL_HPP
#define BINARY_TBL_HPP

/** num個のテーブルtblから key を探す.
 *  @return テーブル中のkeyの位置. 見つからなかった時は num を返す.
 */
template<typename T>
unsigned binary_find_tbl_n(T const* tbl, unsigned num, const T& key)
{
    unsigned    low = 0;
    unsigned    hi  = num;
    while (low < hi) {
        unsigned    mid = (low + hi - 1) / 2;
        if (key < tbl[mid]) {
            hi = mid;
        } else if (tbl[mid] < key) {
            low = mid + 1;
        } else {
            return mid;
        }
    }
    return num;
}

/** テーブルpTblに値keyを追加. 範囲チェックは予め行っていること前提！
 *  @return テーブル中のkeyの位置.
 */
template<typename T>
unsigned binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key) {
    unsigned    hi  = rNum;
    unsigned    low = 0;
    unsigned    mid = 0;
    while (low < hi) {
        mid = (low + hi - 1) / 2;
        if (key < pTbl[mid]) {
            hi = mid;
        } else if (pTbl[mid] < key) {
            ++mid;
            low = mid;
        } else {
            return mid; /* 同じものがみつかったので追加しない */
        }
    }

    // 新規登録
    ++rNum;

    // 登録箇所のメモリを空ける
    for (hi = rNum; --hi > mid;) {
        pTbl[hi] = pTbl[hi-1];
    }

    // 登録
    pTbl[mid] = key;
    return mid;
}

#endif
