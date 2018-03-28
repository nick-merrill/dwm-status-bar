all:
	@cp --no-clobber config.h.def config.h
	@cc -o dwm-status-bar dwm-status-bar.c -O2 -s -lX11

