.PHONY: clean

LIBOPTS = -I/root/torch/install/include -L/root/torch/install/lib -lTH
CFLAGS = -fPIC -shared -g
CC = gcc

TORCSctrl.so : TORCSctrl.cpp
	$(CC) $< $(LIBOPTS) $(CFLAGS) -o $@

clean :
	@rm *.so
