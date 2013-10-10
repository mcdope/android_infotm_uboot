#
# (C) Copyright 2006-2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

CONFIG_NAND_SPL	= y
CONFIG_PRELOADER = y

include $(TOPDIR)/config.mk
include $(TOPDIR)/nand_spl/board/$(BOARDDIR)/config.mk

LDSCRIPT= $(TOPDIR)/nand_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS	= -Bstatic -T $(nandobj)u-boot.lds -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_NAND_SPL
AFLAGS	+= -DCONFIG_PRELOADER
CFLAGS	+= -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_PRELOADER

# essencial: XK
SOBJS	= start.o cpu_init.o lowlevel_init.o
#COBJS-y	+= lowlevel.o
#COBJS-y	+= speed.o
#COBJS-y	+= timer.o
COBJS-y	+= dramc.o
COBJS-y	+= training.o
#COBJS-y	+= train_module.o
COBJS-y	+= boot_main.o
COBJS-y	+= mmc_boot.o
COBJS-y	+= nand_ids.o
COBJS-y	+= nand_uboot0.o
COBJS-y	+= iex_2885.o
COBJS-y	+= iex_2939.o
COBJS-y	+= iex_2974.o
COBJS-y	+= items.o
COBJS-y += imapx800_wdt.o

# div lib: XK
#SOBJS   += _divsi3.o _ashldi3.o _udivsi3.o _lshrdi3.o
#COBJS-y	+= div0.o
#COBJS-y	+= div64.o

# mmc support: 7.5K, depends on div_lib
#COBJS-y	+= mmc.o
#COBJS-y	+= imapx800_sdhc.o

# printf support: 3.5K
#COBJS-y	+= serial_imapx800.o
#COBJS-y	+= vsprintf.o
#COBJS-y	+= ctype.o

#COBJS	= nand_boot.o nand_ecc.o imapx800.o denali_test.o
COBJS	:= $(COBJS-y)

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))

COBJS	:= $(COBJS-y)

OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(OBJTREE)/nand_spl/board/$(BOARDDIR)

nandobj	:= $(OBJTREE)/nand_spl/

ALL	= $(nandobj)u-boot-spl $(nandobj)u-boot-spl.bin

all:	$(obj).depend $(ALL)

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) \
		-Map $(nandobj)u-boot-spl.map \
		-o $(nandobj)u-boot-spl

$(nandobj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links for common files

# from cpu directory
$(obj)start.S:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/start.S $@

$(obj)timer.c:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/imapx800/timer.c $@

$(obj)speed.c:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/imapx800/speed.c $@

$(obj)lowlevel.c:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/imapx800/lowlevel.c $@

$(obj)dramc.c:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/imapx800/dramc.c $@

$(obj)training.c:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/imapx800/training.c $@

$(obj)train_module.c:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/imapx800/train_module.c $@


# from SoC directory
$(obj)cpu_init.S:
	@rm -f $@
	@ln -s $(TOPDIR)/cpu/armv7/imapx800/cpu_init.S $@

# from board directory
$(obj)lowlevel_init.S:
	@rm -f $@
	@ln -s $(TOPDIR)/board/infotm/imapx/lowlevel_init.S $@

# from driver directory
$(obj)serial_imapx800.c:
	@rm -f $@
	@ln -s $(TOPDIR)/drivers/serial/serial_imapx800.c $@

$(obj)mmc.c:
	@rm -f $@
	@ln -s $(TOPDIR)/drivers/mmc/mmc.c $@

$(obj)imapx800_sdhc.c:
	@rm -f $@
	@ln -s $(TOPDIR)/drivers/mmc/imapx800_sdhc.c $@

$(obj)nand_ids.c:
	@rm -f $@
	@ln -s $(TOPDIR)/infotm/drivers/nand/nand_ids.c $@

$(obj)nand_uboot0.c:
	@rm -f $@
	@ln -s $(TOPDIR)/infotm/drivers/nand/nand_uboot0.c $@

$(obj)imapx800_wdt.c:
	@rm -f $@
	@ln -s $(TOPDIR)/infotm/drivers/watchdog/imapx800_wdt.c $@


# from strategy
$(obj)items.c:
	@rm -f $@
	@ln -s $(TOPDIR)/infotm/strategy/items/items.c $@

# from lib_arm
$(obj)_divsi3.S:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_arm/_divsi3.S $@

$(obj)_ashldi3.S:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_arm/_ashldi3.S $@

$(obj)_udivsi3.S:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_arm/_udivsi3.S $@

$(obj)_lshrdi3.S:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_arm/_lshrdi3.S $@

$(obj)div0.c:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_arm/div0.c $@

$(obj)div64.c:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_generic/div64.c $@

$(obj)vsprintf.c:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_generic/vsprintf.c $@

$(obj)ctype.c:
	@rm -f $@
	@ln -s $(TOPDIR)/lib_generic/ctype.c $@

ifneq ($(OBJTREE),$(SRCTREE))
$(obj)preloader.h:
	@rm -f $@
	@ln -s $(TOPDIR)/include/infotm/preloader.h $@

$(obj)boot_main.c: $(obj)preloader.h
	@rm -f $@
	@ln -s $(src)boot_main.c $@

$(obj)mmc_boot.c: $(obj)preloader.h
	@rm -f $@
	@ln -s $(src)mmc_boot.c $@

$(obj)iex_2885.c: $(obj)preloader.h
	@rm -f $@
	$(TOPDIR)/tools/gen-entry.sh 2885 $(src)labels $(SRCTREE)/iromasm/i2885.S > $@

$(obj)iex_2939.c: $(obj)preloader.h
	@rm -f $@
	$(TOPDIR)/tools/gen-entry.sh 2939 $(src)labels $(SRCTREE)/iromasm/i2939.S > $@

$(obj)iex_2974.c: $(obj)preloader.h
	@rm -f $@
	$(TOPDIR)/tools/gen-entry.sh 2974 $(src)labels $(SRCTREE)/iromasm/i2974.S > $@
endif

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
