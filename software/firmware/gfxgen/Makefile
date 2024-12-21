LDLIBS=-lpng

imgdata.c : imggen imgdata.png
	./imggen

imggen : imggen.c

clean :
	$(RM) imggen imgdata.c spritedata.h *~
