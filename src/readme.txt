DiaKbdMouse のソース

■ 使い方や利用条件とかは ../DiaKbdMouse.htm をみてください。

■ Visual Studio 2008(vc9) でコンパイルしてます。
   一応 vs2005(vc8) 用の sln も用意してます(が1,2度コンパイル試したのみ)

※ http://amaisaeta.seesaa.net/article/26290510.html の vcslncnv を用いれば、
   vc8 の sln を vs2002(vc7), vs2003(vc7.1) 用にも変換できるハズ(今回未確認).
   出力フォルダ名等にコンパイラ名を付加してるので変換したら変名推奨.
   (sln, vcprojをテキストエディタで開けて _vc8 等を一括置換すれば)

■ 主に参考にした頁(MSは当然として、その他)
これらの他にも直接的にありがたかった頁があったはずだが控え忘れ.

・キーののっとり方法(dll)
http://hitokuso.kicks-ass.org/progtips.html

・トレイアイコンの処理、CRTを使わないDLL とかのさまざまな小技
http://hp.vector.co.jp/authors/VA000092/win32/index.html

・トレイアイコン
http://www31.ocn.ne.jp/~yoshio2/vcmemo17-1.html

・CapsLock とのキー交換
http://www.jaist.ac.jp/~fujieda/scancode.html
http://www.losttechnology.jp/Tips/keyscancodemap.html


■履歴
2006-??-??  キーボードだけでマウスを使いたい場合があるが、
	    Win標準のキーボードマウスは操作感が悪くてやだったので
	    KbdMouse(v0.5)を作成.
2006-10-??  なんとなくできそうなので、ダイアモンドカーソルを使える版に
	    改造し DiaCursor(v0.5)に改名
2007-05-??  右WIN＋カーソルでの移動で、先にRWinを離すと暴発したのを修正.
200?-??-??  右WINとは別に左WINでマウス操作可能だったが不安定要因なので廃止.
2013-09-??  こまごま色々作業.
	    ・名前を DiaKbgMouse に改名.
	    ・定義ファイル読み込みの実装.
	      これにともない PrivateProfile(.ini)による右WIN変更設定廃止.
	      DiaKbdMouse.exeと同じフォルダにある 同.cfg を読み込む.
	    ・win7/8で機能しない シフト同時押し状態キー(右WIN+B)の廃止.
	    ・右WINキーをやめAPPSキーを使うように変更.
	      ※win7/8では(右)WINキー+L でロック画面へ移行するが、
		乗っ取れず誤操作で右WINキー+Lをよく押してしまったため.
2013-09-22  ・定義ファイルをミスって2ストロークキーが効かなくなってたのを修正.
	    　(ミスしやすい仕様だったのでプログラム側で対処)
	    ・定義ファイルの ^QS(HOME) ^QD(END) を復活.
2015-04-20  ・SendInput するキー設定の dwFlag に KEYEVENTF_EXTENDEDKEY
	      を付けてなかったのを修正
