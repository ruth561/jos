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
#define DEFINE_INT_HANDLER(name)		NL \
	.text					NL \
	.global name##_int_handler		NL \
name##_int_handler:				NL \
	pushq	$0xE22C0DE			NL \
	pushq	%r15				NL \
	pushq	%r14				NL \
	pushq	%r13				NL \
	pushq	%r12				NL \
	pushq	%r11				NL \
	pushq	%r10				NL \
	pushq	%r9				NL \
	pushq	%r8				NL \
	pushq	%rbp				NL \
	pushq	%rsi				NL \
	pushq	%rdi				NL \
	pushq	%rdx				NL \
	pushq	%rcx				NL \
	pushq	%rbx				NL \
	pushq	%rax				NL \
	movq	%rsp, %rdi			NL \
	subq	$8, %rsp			NL \
	call	do_##name##_int_handler		NL \
	addq	$8, %rsp			NL \
	popq	%rax				NL \
	popq	%rbx				NL \
	popq	%rcx				NL \
	popq	%rdx				NL \
	popq	%rdi				NL \
	popq	%rsi				NL \
	popq	%rbp				NL \
	popq	%r8 				NL \
	popq	%r9 				NL \
	popq	%r10				NL \
	popq	%r11				NL \
	popq	%r12				NL \
	popq	%r13				NL \
	popq	%r14				NL \
	popq	%r15				NL \
	addq	$8, %rsp			NL \
	iretq
