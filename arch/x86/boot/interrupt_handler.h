#pragma once

// このファイルはinterrupt_handler.Sのみからincludeされる
// ヘッダファイルである。ここでは、DEFINE_INT_HANDLER(name)
// というマクロを定義しており、interrupt.cで定義されている
// 同名のマクロとセットで使われる。
// 
// 例えば、#PF例外のハンドラを実装したい場合は、まず
// interrupt_handler.Sに
//	DEFINE_INT_HANDLER(pf)
// のように記述する。この記述により、pf_int_handlerという
// 関数が実装される。


// Cプリプロセッサでマクロを展開すると、改行文字などが空白文字
// に置き換えられてしまい、アセンブルできない。そのため、
// `;`という特殊文字によって改行をアセンブラに知らせてあげる
// 必要がある。
#define NL ;

// name_int_handlerという名前の関数を実装する。
// ここで定義された関数は割り込みハンドラとして使われる。
// ※ エラーコードを受け取らないもの限定
// この関数が担っている処理はレジスタの退避であり、
// ハンドラのメイン処理部分はdo_name_int_handlerに実装する。
#define DEFINE_INT_HANDLER(vector)		NL \
	.text					NL \
	.global int##vector##_handler		NL \
int##vector##_handler:				NL \
	pushq	$0xE22C0DE			NL \
	pushq	$vector				NL \
	jmp	common_int_handler

// エラーコードを受け取るハンドラの実装。
// 実装は関数の最初に0xE22C0DEという値をスタックに
// プッシュするか、の違いしかない。
#define DEFINE_INT_HANDLER_WITH_ERR_CODE(vector)	NL \
	.text						NL \
	.global int##vector##_handler			NL \
int##vector##_handler:					NL \
	pushq	$vector					NL \
	jmp	common_int_handler

