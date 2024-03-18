#include "logger.h"
#include "serial.h"


int current_log_level = LOG_LEVEL_DEBUG;

void logger_init(int log_level)
{
	if (log_level < LOG_LEVEL_DEBUG ||
	    log_level > LOG_LEVEL_FATAL) {
		FATAL("Invalid log level: %d", log_level);
		return;
	}
	current_log_level = log_level;
}
