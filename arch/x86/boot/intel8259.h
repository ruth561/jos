#pragma once


// 引数のirqは抽象化されたirqではなく、intel8259につながっている
// 物理的な割り込み線の番号である、ということに注意！
void intel8259_end_of_interrupt(int irq);

void intel8259_init();
