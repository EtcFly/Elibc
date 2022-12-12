
# libs Makefile
include ../Makefile.common
CFLAGSlib+= -c -D"assert_param(expr)=((void)0)"
CFLAGSlib += -Wall -Werror=unused-const-variable=0

LC_SRC = $(notdir $(wildcard src/*.c))
LC_OBJ = $(LC_SRC:%.c=%.o)

LC_INCLUDE = -Isrc/inc

all: libc

libc:libglibc.a

libglibc.a:
	@echo "Building $@ ..."
	@cd src/ && $(CC) $(LC_INCLUDE) $(CFLAGSlib) $(LC_SRC)
	@$(AR) rcsv $(LIBCDIR)/$@ $(LIBCDIR)/src/*.o
	@chmod ugo+x $@
	@echo "done."
.PHONY: libs clean tshow

clean:
	rm -f $(LIBCDIR)/src/*.o
	rm -f $(LIBS)
	rm -f libglibc.a
tshow:
	@echo "######################################################################################################"
	@echo "################# optimize settings: $(InfoTextLib), $(InfoTextSrc)"
	@echo "######################################################################################################"
