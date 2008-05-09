CC = @PSP_CC@
CCAS = $(CC)
AR = @PSP_AR@
RANLIB = @PSP_RANLIB@

EXEEXT =
AM_CPPFLAGS = -I$(srcdir)/ -I$(srcdir)/include -I$(srcdir)/include/freetype2 -I$(top_srcdir)/lib/ -I$(PSPSDK)/include

if OPTIMIZE
AM_CFLAGS = -O3 -G0 -fomit-frame-pointer -ffast-math -frename-registers -Wall
else
AM_CFLAGS = -O0 -G0 -g -Wall
endif
CCASFLAGS = $(CFLAGS) $(INCLUDES)

xReaderdir = $(srcdir)/
xReader_PROGRAMS = xReader.elf

xReader_elf_SOURCES = array.c array.h avc.c avc.h bg.c bg.h bookmark.c bookmark.h \
buffer.c buffer.h charsets.c charsets.h conf.c conf.h copy.c copy.h \
ctrl.c ctrl.h dbg.c dbg.h dictionary.c dictionary.h display.c display.h \
fat.c fat.h fs.c fs.h html.c html.h \
image.c image.h iniparser.c iniparser.h location.c location.h lyric.c lyric.h \
main.c mp3.c mp3.h mp3info.c mp3info.h \
power.c power.h pspscreen.h scene.c scene.h scene_image.c scene_impl.h \
scene_music.c scene_text.c strsafe.c strsafe.h text.c \
text.h ttfont.c ttfont.h usb.c usb.h version.h win.c win.h \
common/datatype.h common/psp_utils.c common/psp_utils.h \
common/qsort.c common/qsort.h common/utils.c common/utils.h \
apetaglib/APETag.c apetaglib/APETag.h apetaglib/TagError.c \
apetaglib/TagHeader.h apetaglib/TagItem.c apetaglib/TagItem.h \
$(top_srcdir)/xrPrx/xrPrx.S simple_gettext.h 

if NLS
xReader_elf_SOURCES += simple_gettext.c sofile.c sofile.h hash.c hash.h
endif

clean-local:
	$(RM) -rf __SCE__xReader %__SCE__xReader xReader xReader% xReader.prx \
		PARAM.SFO EBOOT.PBP

PSP_FW_VERSION = 390
BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

libdir = $(top_srcdir)/lib
LIBS = $(libdir)/unrar.a $(libdir)/unzip.a $(libdir)/libchm.a $(libdir)/libpng.a \
	$(libdir)/libgif.a $(libdir)/libjpeg.a $(libdir)/libbmp.a $(libdir)/libtga.a \
	$(libdir)/libid3tag.a \
	$(libdir)/libmad.a $(libdir)/libz.a $(libdir)/libfreetype.a $(libdir)/libwmadec.a \
	$(libdir)/pmpmodavc/pmpmodavclib.a $(libdir)/pmpmodavc/libavformat.a $(libdir)/pmpmodavc/libavcodec.a $(libdir)/pmpmodavc/libavutil.a $(libdir)/pmpmodavc/libpspmpeg.a $(libdir)/pmpmodavc/pmpmodavclib.a $(libdir)/libpspsystemctrl_kernel.a  \
	-L$(libdir) -lexif -lm -lpspaudio -lpspaudiolib -lpspgu -lpspgum -lpsphprm -lpsppower -lpsprtc \
	-lpspusb -lpspusbstor -lstdc++ -lpspkubridge -lpspaudio -lpspaudiocodec -lpspaudio -lpspmpeg 

TARGET = xReader
EXTRA_TARGETS = EBOOT.PBP
EXTRA_CLEAN = -r __SCE__xReader %__SCE__xReader xReader xReader% xReader.prx
PSP_EBOOT_TITLE = xReader
PSP_EBOOT_ICON = ../resource/ICON0.png
include $(PSPSDK)/lib/build.mak
