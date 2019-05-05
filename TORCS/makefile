.PHONY: clean

LIBOPTS = -I/home/hxu/torch/install/include -L/home/hxu/torch/install/lib -lTH
CFLAGS = -fPIC -shared -g
CC = gcc

TORCSctrl.so : TORCSctrl.cpp
	$(CC) $< $(LIBOPTS) $(CFLAGS) -o $@

clean :
	@rm *.so
