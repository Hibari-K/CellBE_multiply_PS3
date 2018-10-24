PPUCC	= ppu-gcc
EMB		= ppu-embedspu
SPUCC	= spu-gcc
CFLAGS	= -g -lspe2

SPUPROG	= spu_split
SPUPROG2	= spu_kernel
PROGRAM	= ppu_main
SPUOBJ	= $(SPUPROG).o
SPUOBJ2	= $(SPUPROG2).o
SPUALL	= $(SPUOBJ) $(SPUOBJ2)
PPUOBJ	= $(PROGRAM).o

all: $(PROGRAM)

$(PROGRAM): $(SPUALL)
	$(PPUCC) -o $(PROGRAM) $(PROGRAM).c $(SPUOBJ) $(SPUOBJ2) $(CFLAGS)

$(SPUOBJ):
	$(SPUCC) $(SPUPROG).c -o $(SPUPROG)
	$(EMB) -m64 $(SPUPROG) $(SPUPROG) $(SPUOBJ)

$(SPUOBJ2):
	$(SPUCC)  $(SPUPROG2).c -o $(SPUPROG2)
	$(EMB) -m64 $(SPUPROG2) $(SPUPROG2) $(SPUOBJ2)

clean:
	rm $(PROGRAM) $(SPUPROG) *.o
