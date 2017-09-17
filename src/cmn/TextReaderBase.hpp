/**
 *	@file	TextReaderBase.h
 *	@brief	�o�C�g����e�L�X�g�Ƃ��� �����E�s�ǂݍ��݂��邽�߂̊��N���X
 *	@note
 *	- \r\n �� \n ������.
 *	- �N���X�p������ raw_close(), raw_read() (�ꍇ�ɂ���Ă� raw_malloc, raw_free)
 *	  �� �I�[�o�[���C�h���Ď����̂���.
 */
#ifndef TEXTREADERBASE_H
#define TEXTREADERBASE_H

#include <cstddef>
#include <cassert>

// �o�C�g����e�L�X�g�Ƃ��� �����E�s�ǂݍ��݂��邽�߂̊��N���X
class TextReaderBase {
	enum { INNER_BUF_SZ = 3 };	// ����p�f�B���O���p�̂�.
public:
	typedef std::size_t		size_t;
	TextReaderBase() { init(); }
	TextReaderBase(char* buf, unsigned sz) { init(); setbuf(buf,sz); }
	~TextReaderBase() { close(); }

	void		close() {
					raw_close();
					if (alcFlag_)
						raw_free(buf_);
				}

	size_t		read(void* readbuf, size_t size) {
					return readEx(readbuf, size, false);
				}

	int 		getc() {
					char	buf[2];
					return read(buf, 1) > 0 ? buf[0] : -1;
				}

	char*		gets(char* buf, size_t bufSize) {
				 #if 1
					assert(buf && bufSize > 1);
					std::size_t rdsz = readEx(buf, bufSize-1, true);
					if (std::ptrdiff_t(rdsz) > 0) {
						buf[rdsz] = 0;
						return buf;
					}
					return NULL;
				 #else
					char*	d  = buf;
					assert(buf);
					if (bufSize > 0) {
						char*	de = d + bufSize - 1;
						if (d < de) {
							char	buf[2];
							int c;
							do {
								if (read(buf, 1) <= 0)
									break;
								c = buf[0];
								*d++ = c;
							} while (d < de && c != '\n' /*&& c != '\r'*/);
						}
						*d = 0;
					}
					return buf;
				 #endif
				}

	int			setbuf(char* buf, size_t size) {
					int		rc = 0;
					if (buf_) {
						// flush();
						if (alcFlag_)
							raw_free(buf_);
						buf_ 	 = 0;
						alcFlag_ = 0;
					}
					if (size >= 0x80000000)	// �o�b�t�@�T�C�Y��int�̐����͈�.
						goto ERR;
					if (buf) {	// �Ăь����p�ӂ����o�b�t�@������Ƃ�
						if (size < 2)	// 2�����Ȃ�����̂��g��.
							goto ERR;
					} else if (size <= INNER_BUF_SZ) {
						buf			= innerBuf_;
						size 		= INNER_BUF_SZ;
					} else {
						size		= size;
						buf			= (char*) raw_malloc( size );
						if (buf) {
							alcFlag_ = 1;
						} else {	// allocate�Ɏ��s����������o�b�t�@���g��.
				  ERR:
							buf		= innerBuf_;
							size 	= INNER_BUF_SZ;
							rc 		= -1;
						}
					}
					buf_		= buf;
					bufCapa_	= size;
					bufCur_		= 0;
					bufSize_	= 0;
					return rc;
				}

private:
	size_t		readEx(void* readbuf, size_t size, bool crFlag) {
					assert(readbuf != NULL);
					assert(buf_ != NULL);
					assert(bufCapa_ > 0);
					assert(bufSize_ <= bufCapa_);
					assert(bufCur_  <= bufSize_);

					error_	= 0;

					size_t	total 		= 0;
					char*	dst			= (char*)readbuf;
					char*	buf			= buf_;
					size_t	bufCapa		= bufCapa_;
					while (size > 0) {
						std::ptrdiff_t	r = bufSize_ - bufCur_;
						assert(r >= 0);
						if (r > 0) {	// �o�b�t�@�Ƀf�[�^������΂܂��͂�������擾.
							size_t l;
							l = size;
							if (l > size_t(r))
								l = r;
							if (crFlag == 0) {
								std::memcpy(dst, buf+bufCur_, l);
								//std::copy(dst, dst+l, buf+bufCur_);
								dst 		+= l;
							} else {
								char* s  = buf+bufCur_;
								char* se = s + l;
								while (s < se) {
									int c  = *s++;
									*dst++ = c;
									if (c == '\n') {
										l    = s - (buf+bufCur_);
										size = l;
										break;
									}
								}
							}
							bufCur_ 	+= l;
							total		+= l;
							size		-= l;
							if (size == 0)
								break;
						}
						bufCur_  		= 0;
						bufSize_ 		= 0;
						if (eof_) {	// �Ō�܂œǂݏI����Ă����ꍇ�͏I��.
							break;
						}

						// �t�@�C������o�b�t�@�ɓǂݍ���.
						if (restCR_) {	// �O��̍ŌオCR�������ꍇ�̒���.
							restCR_ = 0;
							buf[0]	= '\r';
							r = raw_read(buf+1, bufCapa-1);
							if (r >= 0)
								++r;
						} else {
							r = raw_read(buf, bufCapa);
						}
						if (r <= 0) {		// �ǂݍ��߂Ȃ�����.
							if (r < 0) {	// ���Ȃ�G���[����.
								error_	= 1;
								total	= (size_t)-1;
							}
							break;
						}
						if (r < std::ptrdiff_t(bufCapa)) {
							eof_	= 1;
						}
						bufSize_	= r;
						if (r > 0) {	// �f�[�^��ǂݍ��߂Ă����� ���s�R�[�h�̕ϊ����s��.
							const char* s  = buf;
							const char* se = buf + r;
							char*		d  = buf;
							do {
								int c = *s++;
								if (c == '\r') {		 // \r���������� �����\n�ƈ�̉�����.
									if (s < se) {
										if (*s == '\n') {	// \r\n�Ȃ� \n��.
											c = '\n';
											++s;
										}
									} else {			 // �o�b�t�@�̍Ō�1�o�C�g��\r�̂Ƃ�.
										if (eof_ == 0) { // EOF���܂��Ȃ�
											restCR_ = 1; // \r������Ɏ����z��.
											break;
										}				 // EOF���͂��̂܂܏���.
									}
								}
								*d++ = c;
							} while (s < se);
							bufSize_ = d - buf;
						}
					}
					return total;
				}

	void		init() {
					buf_ 	 = NULL;
					bufSize_ = 0;
					bufCur_  = 0;
					bufCapa_ = 0;
					alcFlag_ = 0;
					error_   = 0;
					eof_	 = 0;
					restCR_	 = 0;
				}

protected:
	virtual void 	raw_close() {}	// = 0;
	virtual size_t	raw_read(void* buf, size_t size) = 0;
	virtual void*	raw_malloc(size_t) { return NULL; }
	virtual void    raw_free(void*) { }

private:
	char*			buf_;						// �o�b�t�@�擪
	unsigned		bufSize_;					// �ǂݍ���ł���f�[�^�̃T�C�Y
	unsigned		bufCur_;					// ���݂̏����ʒu
	unsigned		bufCapa_;					// �o�b�t�@�̗̈�T�C�Y
	unsigned char	alcFlag_: 1;
	unsigned char	error_	: 1;
	unsigned char	eof_	: 1;
	unsigned char	restCR_ : 1;
	char			innerBuf_[INNER_BUF_SZ];	// �����o�b�t�@. ����G���[���̍ň����p.
};

#endif
