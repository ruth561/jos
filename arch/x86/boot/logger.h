#pragma once

// デバッグメッセージを出力する関数・マクロを定義しているファイル。
// シリアルコンソールの初期化が完了している必要がある。
// 書き出したデバッグメッセージは、シリアルポートを経由して渡される
// ことになっている。
//
// デバッグメッセージに含めたい情報としては、
// 	[時刻] [デバッグレベル] デバッグの内容 (デバッグのソースコードの位置)
// 
// ただし、時刻に関してはタイマーの初期化が完了していないので、
// まだ実装しない（ＴＯＤＯ：）
//
// 局所的にデバッグレベルを変動させたいときがある。そのため、デバッグ
// レベルは動的に変更できるようにしたほうがいい？

#include "console.h"
#include "type.h"

#define LOG_LEVEL_DEBUG	0
#define LOG_LEVEL_INFO	1
#define LOG_LEVEL_WARN	2
#define LOG_LEVEL_ERROR	3
#define LOG_LEVEL_FATAL	4

extern int current_log_level;

// ログレベルを設定する関数。
// 返り値には、もとのログレベルが返される。
inline int set_log_level(int log_level)
{
	int retval = current_log_level;
	current_log_level = log_level;
	return retval;
}

#define WRITE_LOG(log_level, head, ...)				\
	if (log_level >= current_log_level) {			\
		prints(head);					\
		prints(__VA_ARGS__);				\
		prints(" (%s:%d)\r\n", __FILE__, __LINE__);	\
	}

#define DEBUG(...) WRITE_LOG(LOG_LEVEL_DEBUG, "[ DEBUG ] ", __VA_ARGS__)
#define  INFO(...) WRITE_LOG( LOG_LEVEL_INFO, "[  INFO ] ", __VA_ARGS__)
#define  WARN(...) WRITE_LOG( LOG_LEVEL_WARN, "[  WARN ] ", __VA_ARGS__)
#define ERROR(...) WRITE_LOG(LOG_LEVEL_ERROR, "[ ERROR ] ", __VA_ARGS__)
#define FATAL(...) WRITE_LOG(LOG_LEVEL_FATAL, "[ FATAL ] ", __VA_ARGS__)

// ロガーの初期化を行う関数。最初に設定するログレベルをlog_levelに指定する。
void logger_init(int log_level);
