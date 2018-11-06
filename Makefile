#  copyright (c) 2018 quackmore-ff@yahoo.com
#
.NOTPARALLEL:

# Local project and SDK reference
TOP_DIR:=$(abspath $(dir $(lastword $(MAKEFILE_LIST))))
CCFLAGS:= -I$(TOP_DIR)/src/include -I$(SDK_DIR)/include
LDFLAGS:= -L$(SDK_DIR)/lib -L$(SDK_DIR)/ld $(LDFLAGS)

ifeq ($(BOOT), new)
    boot = new
else
    ifeq ($(BOOT), old)
        boot = old
    else
        boot = none
    endif
endif

ifeq ($(APP), 1)
    app = 1
else
    ifeq ($(APP), 2)
        app = 2
    else
        app = 0
    endif
endif

ifeq ($(SPI_SPEED), 26.7)
    freqdiv = 1
else
    ifeq ($(SPI_SPEED), 20)
        freqdiv = 2
    else
        ifeq ($(SPI_SPEED), 80)
            freqdiv = 15
        else
            freqdiv = 0
        endif
    endif
endif

ifeq ($(SPI_MODE), QOUT)
    mode = 1
else
    ifeq ($(SPI_MODE), DIO)
        mode = 2
    else
        ifeq ($(SPI_MODE), DOUT)
            mode = 3
        else
            mode = 0
        endif
    endif
endif

addr = 0x01000

ifeq ($(SPI_SIZE_MAP), 1)
  size_map = 1
  flash = 256
else
  ifeq ($(SPI_SIZE_MAP), 2)
    size_map = 2
    flash = 1024
    ifeq ($(app), 2)
      addr = 0x81000
    endif
  else
    ifeq ($(SPI_SIZE_MAP), 3)
      size_map = 3
      flash = 2048
      ifeq ($(app), 2)
        addr = 0x81000
      endif
    else
      ifeq ($(SPI_SIZE_MAP), 4)
        size_map = 4
        flash = 4096
        ifeq ($(app), 2)
          addr = 0x81000
        endif
      else
        ifeq ($(SPI_SIZE_MAP), 5)
          size_map = 5
          flash = 2048
          ifeq ($(app), 2)
            addr = 0x101000
          endif
        else
          ifeq ($(SPI_SIZE_MAP), 6)
            size_map = 6
            flash = 4096
            ifeq ($(app), 2)
              addr = 0x101000
            endif
          else
            ifeq ($(SPI_SIZE_MAP), 8)
              size_map = 8
              flash = 8192
              ifeq ($(app), 2)
                addr = 0x101000
              endif
            else
              ifeq ($(SPI_SIZE_MAP), 9)
                size_map = 9
                flash = 16384
                ifeq ($(app), 2)
                  addr = 0x101000
                endif
              else
                size_map = 0
                flash = 512
                ifeq ($(app), 2)
                addr = 0x41000
                endif
              endif
            endif
          endif
        endif
      endif
    endif
  endif
endif

LD_FILE = $(LDDIR)/eagle.app.v6.ld

ifneq ($(boot), none)
ifneq ($(app),0)
    ifneq ($(findstring $(size_map),  6  8  9),)
      LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).2048.ld
    else
      ifeq ($(size_map), 5)
        LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).2048.ld
      else
        ifeq ($(size_map), 4)
          LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
        else
          ifeq ($(size_map), 3)
            LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
          else
            ifeq ($(size_map), 2)
              LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
            else
              ifeq ($(size_map), 0)
                LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).512.app$(app).ld
              endif
            endif
          endif
        endif
      endif
    endif
    BIN_NAME = user$(app).$(flash).$(boot).$(size_map)
endif
else
    app = 0
endif

ifdef DEBUG
	CCFLAGS += -ggdb -O0
	LDFLAGS += -ggdb
else
	CCFLAGS += -Os
endif

#############################################################
# Select compile
#
ifeq ($(OS),Windows_NT)
	# WIN32
	# We are under windows.
	ifeq ($(XTENSA_CORE),lx106)
		# It is xcc
		AR = xt-ar
		CC = xt-xcc
		CXX = xt-xcc
		NM = xt-nm
		CPP = xt-cpp
		OBJCOPY = xt-objcopy
        OBJDUMP = xt-objdump
		CCFLAGS += --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal
	else 
		# It is gcc, may be cygwin
		# Can we use -fdata-sections?
		CCFLAGS += -ffunction-sections -fno-jump-tables -fdata-sections
		AR = xtensa-lx106-elf-ar
		CC = xtensa-lx106-elf-gcc
		CXX = xtensa-lx106-elf-g++
		NM = xtensa-lx106-elf-nm
		CPP = xtensa-lx106-elf-cpp
		OBJCOPY = xtensa-lx106-elf-objcopy
    OBJDUMP = xtensa-lx106-elf-objdump
	endif
	FIRMWAREDIR = ..\\bin\\
	ifndef COMPORT
		ESPPORT = com1
	else
		ESPPORT = $(COMPORT)
	endif
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
	# ->AMD64
	endif
	ifeq ($(PROCESSOR_ARCHITECTURE),x86)
	# ->IA32
	endif
else
	# We are under other system, may be Linux. Assume using gcc.
	# Can we use -fdata-sections?
	ifndef COMPORT
		ESPPORT = /dev/ttyUSB0
	else
		ESPPORT = $(COMPORT)
	endif
	CCFLAGS += -ffunction-sections -fno-jump-tables -fdata-sections
	AR = xtensa-lx106-elf-ar
	CC = $(WRAPCC) xtensa-lx106-elf-gcc
	CXX = $(WRAPCC) xtensa-lx106-elf-g++
	NM = xtensa-lx106-elf-nm
	CPP = $(WRAPCC) xtensa-lx106-elf-gcc -E
	OBJCOPY = xtensa-lx106-elf-objcopy
    OBJDUMP = xtensa-lx106-elf-objdump
	FIRMWAREDIR = ../bin/
	UNAME_S := $(shell uname -s)
	
	ifeq ($(UNAME_S),Linux)
	# LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
	# OSX
	endif
	
	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
	# ->AMD64
	endif
	ifneq ($(filter %86,$(UNAME_P)),)
	# ->IA32
	endif
	ifneq ($(filter arm%,$(UNAME_P)),)
	# ->ARM
	endif
endif
#############################################################
ESPTOOL ?= esptool.py


CSRCS ?= $(wildcard *.c)
CXXSRCS ?= $(wildcard *.cpp)
ASRCs ?= $(wildcard *.s)
ASRCS ?= $(wildcard *.S)
SUBDIRS ?= $(patsubst %/,%,$(dir $(filter-out tools/Makefile,$(wildcard */Makefile))))

ODIR := .output
OBJODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/obj

OBJS := $(CSRCS:%.c=$(OBJODIR)/%.o) \
        $(CXXSRCS:%.cpp=$(OBJODIR)/%.o) \
        $(ASRCs:%.s=$(OBJODIR)/%.o) \
        $(ASRCS:%.S=$(OBJODIR)/%.o)

DEPS := $(CSRCS:%.c=$(OBJODIR)/%.d) \
        $(CXXSCRS:%.cpp=$(OBJODIR)/%.d) \
        $(ASRCs:%.s=$(OBJODIR)/%.d) \
        $(ASRCS:%.S=$(OBJODIR)/%.d)

LIBODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/lib
OLIBS := $(GEN_LIBS:%=$(LIBODIR)/%)

IMAGEODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/image
OIMAGES := $(GEN_IMAGES:%=$(IMAGEODIR)/%)

BINODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/bin
OBINS := $(GEN_BINS:%=$(BINODIR)/%)

GIT_VERSION := $(shell git --no-pager describe --tags --always --dirty)

#
# Note: 
# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
# If you add global optimize options like "-O2" here 
# they will override "-Os" defined above.
# "-Os" should be used to reduce code size
#
CCFLAGS += 			\
	-g			\
	-Wpointer-arith		\
	-Wundef			\
	-Werror			\
	-Wl,-EL			\
	-fno-inline-functions	\
	-nostdlib       \
	-mlongcalls	\
	-mtext-section-literals \
	-ffunction-sections \
	-fdata-sections	\
	-fno-builtin-printf \
  -DESPBOT_RELEASE=\"$(GIT_VERSION)\" \
	-DSPI_FLASH_SIZE_MAP=$(SPI_SIZE_MAP)
#	-Wall			

CFLAGS = $(CCFLAGS) $(DEFINES) $(EXTRA_CCFLAGS) $(STD_CFLAGS) $(INCLUDES)
DFLAGS = $(CCFLAGS) $(DDEFINES) $(EXTRA_CCFLAGS) $(STD_CFLAGS) $(INCLUDES)


#############################################################
# Functions
#

define ShortcutRule
$(1): .subdirs $(2)/$(1)
endef

define MakeLibrary
DEP_LIBS_$(1) = $$(foreach lib,$$(filter %.a,$$(COMPONENTS_$(1))),$$(dir $$(lib))$$(LIBODIR)/$$(notdir $$(lib)))
DEP_OBJS_$(1) = $$(foreach obj,$$(filter %.o,$$(COMPONENTS_$(1))),$$(dir $$(obj))$$(OBJODIR)/$$(notdir $$(obj)))
$$(LIBODIR)/$(1).a: $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1)) $$(DEPENDS_$(1))
	@mkdir -p $$(LIBODIR)
	$$(if $$(filter %.a,$$?),mkdir -p $$(EXTRACT_DIR)_$(1))
	$$(if $$(filter %.a,$$?),cd $$(EXTRACT_DIR)_$(1); $$(foreach lib,$$(filter %.a,$$?),$$(AR) xo $$(UP_EXTRACT_DIR)/$$(lib);))
	$$(AR) ru $$@ $$(filter %.o,$$?) $$(if $$(filter %.a,$$?),$$(EXTRACT_DIR)_$(1)/*.o)
	$$(if $$(filter %.a,$$?),$$(RM) -r $$(EXTRACT_DIR)_$(1))
endef

define MakeImage
DEP_LIBS_$(1) = $$(foreach lib,$$(filter %.a,$$(COMPONENTS_$(1))),$$(dir $$(lib))$$(LIBODIR)/$$(notdir $$(lib)))
DEP_OBJS_$(1) = $$(foreach obj,$$(filter %.o,$$(COMPONENTS_$(1))),$$(dir $$(obj))$$(OBJODIR)/$$(notdir $$(obj)))
$$(IMAGEODIR)/$(1).out: $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1)) $$(DEPENDS_$(1))
	@mkdir -p $$(IMAGEODIR)
	$$(CC) $$(LDFLAGS) $$(if $$(LINKFLAGS_$(1)),$$(LINKFLAGS_$(1)),$$(LINKFLAGS_DEFAULT) $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1))) -o $$@ 
endef

$(BINODIR)/%.bin: $(IMAGEODIR)/%.out
	@mkdir -p $(BINODIR)
	
ifeq ($(APP), 0)
	@$(RM) -r ../bin/eagle.S ../bin/eagle.dump
	@$(OBJDUMP) -x -s $< > ../bin/eagle.dump
	@$(OBJDUMP) -S $< > ../bin/eagle.S
else
	mkdir -p ../bin/upgrade
	@$(RM) -r ../bin/upgrade/$(BIN_NAME).S ../bin/upgrade/$(BIN_NAME).dump
	@$(OBJDUMP) -x -s $< > ../bin/upgrade/$(BIN_NAME).dump
	@$(OBJDUMP) -S $< > ../bin/upgrade/$(BIN_NAME).S
endif

	@$(OBJCOPY) --only-section .text -O binary $< eagle.app.v6.text.bin
	@$(OBJCOPY) --only-section .data -O binary $< eagle.app.v6.data.bin
	@$(OBJCOPY) --only-section .rodata -O binary $< eagle.app.v6.rodata.bin
	@$(OBJCOPY) --only-section .irom0.text -O binary $< eagle.app.v6.irom0text.bin

	@echo ""
	@echo "!!!"
	@echo "VERSION: "$(GIT_VERSION)
	
ifeq ($(app), 0)
	@python $(SDK_DIR)/tools/gen_appbin.py $< 0 $(mode) $(freqdiv) $(size_map) $(app)
	@mv eagle.app.flash.bin ../bin/eagle.flash.bin
	@mv eagle.app.v6.irom0text.bin ../bin/eagle.irom0text.bin
	@rm eagle.app.v6.*
	@echo "No boot needed."
	@echo "Generate eagle.flash.bin and eagle.irom0text.bin successully in folder bin."
	@echo "eagle.flash.bin-------->0x00000"
	@echo "eagle.irom0text.bin---->0x10000"
else
    ifneq ($(boot), new)
		@python $(SDK_DIR)/tools/gen_appbin.py $< 1 $(mode) $(freqdiv) $(size_map) $(app)
		@echo "Support boot_v1.1 and +"
    else
		@python $(SDK_DIR)/tools/gen_appbin.py $< 2 $(mode) $(freqdiv) $(size_map) $(app)

    	ifeq ($(size_map), 6)
		@echo "Support boot_v1.4 and +"
        else
            ifeq ($(size_map), 5)
		@echo "Support boot_v1.4 and +"
            else
		@echo "Support boot_v1.2 and +"
            endif
        endif
    endif

	@mv eagle.app.flash.bin ../bin/upgrade/$(BIN_NAME).bin
	@rm eagle.app.v6.*
	@echo "Generate $(BIN_NAME).bin successully in folder bin/upgrade."
	@echo "boot.bin------------>0x00000"
	@echo "(Pick up boot.bin in folder $(SDK_DIR)/bin)"
	@echo "$(BIN_NAME).bin--->$(addr)"
endif

	@echo "!!!"
#############################################################
# Rules base
# Should be done in top-level makefile only
#

all:	.subdirs $(OBJS) $(OLIBS) $(OIMAGES) $(OBINS) $(SPECIAL_MKTARGETS)
ifndef SDK_DIR
  	$(error SDK_DIR is undefined. Generate the environment variables using 'gen_env.sh' or load them with '. env.sh')
endif


clean:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clean;)
	$(RM) -r $(ODIR)/$(TARGET)/$(FLAVOR)
	$(RM) -r "$(TOP_DIR)/sdk"

clobber: $(SPECIAL_CLOBBER)
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clobber;)
	$(RM) -r $(ODIR)

flash:
	@echo "use one of the following targets to flash the firmware"
	@echo "  make flash512k - for ESP with 512kB flash size"
	@echo "  make flash4m   - for ESP with   4MB flash size"

flash512k:
	$(MAKE) -e FLASHOPTIONS="-fm qio -fs  4m -ff 40m" flashinternal

flash4m:
	$(MAKE) -e FLASHOPTIONS="-fm dio -fs 32m -ff 40m" flashinternal

flashinternal:
ifndef PDIR
	$(MAKE) -C ./app flashinternal
else
	$(ESPTOOL) --port $(ESPPORT) write_flash $(FLASHOPTIONS) 0x00000 $(FIRMWAREDIR)0x00000.bin 0x10000 $(FIRMWAREDIR)0x10000.bin
endif

.subdirs:
	@set -e; $(foreach d, $(SUBDIRS), $(MAKE) -C $(d);)

#.subdirs:
#	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d))

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),clobber)
ifdef DEPS
sinclude $(DEPS)
endif
endif
endif


$(OBJODIR)/%.o: %.c
	@mkdir -p $(OBJODIR);
	$(CC) $(if $(findstring $<,$(DSRCS)),$(DFLAGS),$(CFLAGS)) $(COPTS_$(*F)) -o $@ -c $<

$(OBJODIR)/%.d: %.c
	@mkdir -p $(OBJODIR);
	@echo DEPEND: $(CC) -M $(CFLAGS) $<
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJODIR)/%.o: %.cpp
	@mkdir -p $(OBJODIR);
	$(CXX) $(if $(findstring $<,$(DSRCS)),$(DFLAGS),$(CFLAGS)) $(COPTS_$(*F)) -o $@ -c $<

$(OBJODIR)/%.d: %.cpp
	@mkdir -p $(OBJODIR);
	@echo DEPEND: $(CXX) -M $(CFLAGS) $<
	@set -e; rm -f $@; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJODIR)/%.o: %.s
	@mkdir -p $(OBJODIR);
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJODIR)/%.d: %.s
	@mkdir -p $(OBJODIR); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJODIR)/%.o: %.S
	@mkdir -p $(OBJODIR);
	$(CC) $(CFLAGS) -D__ASSEMBLER__ -o $@ -c $<

$(OBJODIR)/%.d: %.S
	@mkdir -p $(OBJODIR); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(foreach lib,$(GEN_LIBS),$(eval $(call ShortcutRule,$(lib),$(LIBODIR))))

$(foreach image,$(GEN_IMAGES),$(eval $(call ShortcutRule,$(image),$(IMAGEODIR))))

$(foreach bin,$(GEN_BINS),$(eval $(call ShortcutRule,$(bin),$(BINODIR))))

$(foreach lib,$(GEN_LIBS),$(eval $(call MakeLibrary,$(basename $(lib)))))

$(foreach image,$(GEN_IMAGES),$(eval $(call MakeImage,$(basename $(image)))))

#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := $(INCLUDES) -I $(PDIR)include -I $(PDIR)include/$(TARGET)
PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile
