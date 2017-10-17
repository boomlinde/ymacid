bin = YMACID.EXE
src = $(patsubst %,build/%,$(wildcard *.c))
header = $(patsubst %,build/%,$(wildcard *.h))

build/$(bin): $(src) $(header) build/Makefile dos/tc build/ymacid.cfg
	SDL_VIDEODRIVER=dummy dosbox -conf compile.conf
	dos2unix build/MKDEBUG.TXT
	cat build/MKDEBUG.TXT
	@! cat build/MKDEBUG.TXT |grep '** error' >/dev/null
	@! cat build/MKDEBUG.TXT |grep 'Fatal:' >/dev/null

run: build/$(bin)
	dosbox -conf run.conf

build/Makefile: Makefile.dos build
	unix2dos -n Makefile.dos build/Makefile

build/%.c: build %.c
	unix2dos -n $*.c $@

build/%.h: build %.h
	unix2dos -n $*.h $@

build/ymacid.cfg: build ymacid.cfg
	unix2dos -n ymacid.cfg build/ymacid.cfg

build:
	mkdir build

clean:
	-rm -rf build
