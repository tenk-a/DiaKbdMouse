#ifndef BINARY_TBL_HPP
#define BINARY_TBL_HPP

template<typename T>
unsigned binary_find_tbl_n(T const* tbl, unsigned num, const T& key)
{
	unsigned	low = 0;
	unsigned    hi  = num;
	while (low < hi) {
		unsigned	mid = (low + hi - 1) / 2;
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

/** �e�[�u��pTbl�ɒlkey��ǉ�. �͈̓`�F�b�N�͗\�ߍs���Ă��邱�ƑO��I
 *  @return �e�[�u������key�̈ʒu.
 */
template<typename T>
unsigned binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key) {
	unsigned 	hi  = rNum;
	unsigned 	low = 0;
	unsigned 	mid = 0;
	while (low < hi) {
		mid = (low + hi - 1) / 2;
		if (key < pTbl[mid]) {
			hi = mid;
		} else if (pTbl[mid] < key) {
			++mid;
			low = mid;
		} else {
			return mid;	/* �������̂��݂������̂Œǉ����Ȃ� */
		}
	}

	// �V�K�o�^
	++rNum;

	// �o�^�ӏ��̃��������󂯂�
	for (hi = rNum; --hi > mid;) {
		pTbl[hi] = pTbl[hi-1];
	}

	// �o�^
	pTbl[mid] = key;
	return mid;
}

#endif
