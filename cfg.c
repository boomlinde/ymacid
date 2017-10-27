#include "cfg.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "fm.h"
#include "theme.h"
#include "tick.h"

static int config_next(const char **p, char *out)
{
	static int len;

	for (len = 0;; (*p)++) {
		if (strchr("\n\r\t ", **p) || **p == 0) {
			if (len) {
				out[len] = 0;
				return 1;
			} else if (**p == 0) return 0;
		} else out[len++] = **p;
	}
}

static int config_read(const char *data)
{
#define NAME(_name) !strcmp(buf, #_name)
#define NEXT(_ret) { if (!config_next(&data, buf)) return _ret; }

	static char buf[20];

	for (;;) {
		NEXT(1);
		if (NAME(theme)) {
			NEXT(0);
			if (!strcmp(buf, "acid")) {
				theme_cur = &theme_acid;
			} else if (!strcmp(buf, "data")) {
				theme_cur = &theme_data;
			}
		} else if (NAME(port)) {
			NEXT(0);
			fm_port = strtol(buf, 0, 0);
		} else if (NAME(opl3)) {
			NEXT(0);
			fm_opl3 = strtol(buf, 0, 0);
		} else if (NAME(clock)) {
			NEXT(0);
			if (!strcmp(buf, "pit")) {
				tick_mode = TICK_PCSPKR;
			} else if (!strcmp(buf, "int")) {
				tick_mode = TICK_INT15H;
			}
		} else if (NAME(split)) {
			NEXT(0);
			fm_split = strtol(buf, 0, 0);
		}
	}
#undef NAME
#undef NEXT
}

int config_load(const char *fname)
{
	static FILE *f;
	static int fsize;
	static char *data;
	static int ret;

	f = fopen(fname, "r");
	if (!f)                    return 1;
	if (fseek(f, 0, SEEK_END)) return fclose(f) || 1;

	fsize = ftell(f);
	if (fsize == -1)           return fclose(f) || 1;
	if (fseek(f, 0, SEEK_SET)) return fclose(f) || 1;

	data = malloc(fsize + 1);
	if (!data) return fclose(f) || 1;
	if (!fread(data, 1, fsize, f)) {
		free(data);
		return fclose(f);
	};

	data[fsize] = 0;

	ret = config_read(data);
	free(data);
	return ret;
}
