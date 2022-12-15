
# libs Makefile
include ../Makefile.common
CFLAGSlib+= -c -D"assert_param(expr)=((void)0)"
CFLAGSlib += -Wall -Werror=unused-const-variable=0

LC_SRC = $(shell find . -name "*.c" -type f)
LC_OBJ = $(LC_SRC:%.c=%.o)

LC_INCLUDE = -I$(LIBCDIR)/src/inc

all: libc

libc:libglibc.a

libglibc.a:
	@echo "Building $@ ..."
	@cd src/stdio && $(CC) $(LC_INCLUDE) $(CFLAGSlib) *.c
	@cd src/misc && $(CC) $(LC_INCLUDE) $(CFLAGSlib) *.c
	@cd src/string && $(CC) $(LC_INCLUDE) $(CFLAGSlib) *.c
	@cd src/stdlib && $(CC) $(LC_INCLUDE) $(CFLAGSlib) *.c
	@cd src/ && $(CC) $(LC_INCLUDE) $(CFLAGSlib) *.c
	# Generate the generic  library
	@$(AR) rcsv $(LIBCDIR)/$@ $(LC_OBJ)
	@chmod ugo+x $@
	@echo "done."
.PHONY: libs clean tshow

clean:
	-rm -f $(LIBCDIR)/src/*.o
	-rm -f $(LIBCDIR)/src/misc/*.o
	-rm -f $(LIBCDIR)/src/string/*.o
	-rm -f $(LIBCDIR)/src/stdio/*.o
	-rm -f $(LIBCDIR)/src/stdlib/*.o
	-rm -f $(LIBCDIR)/*.o
	-rm -f $(LIBS)
	-rm -f libglibc.a
tshow:
	@echo "######################################################################################################"
	@echo "################# optimize settings: $(InfoTextLib), $(InfoTextSrc)"
	@echo "######################################################################################################"
