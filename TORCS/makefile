.PHONY: clean

LIBOPTS = -I/data/hxu/code/torch/install/include -L/data/hxu/code/torch/install/lib -lTH
CFLAGS = -fPIC -shared -g
CC = gcc

TORCSctrl.so : TORCSctrl.cpp
	$(CC) $< $(LIBOPTS) $(CFLAGS) -o $@

clean :
	@rm *.so
