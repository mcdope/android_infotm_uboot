#include <common.h>
#include <oem_graphic.h>
#include <oem_pic_data.h>
/* Logo */
struct oem_picture oem_pic_logo = {
	0,	0,	0x118,	0x9f,	"InfoTM LOGO",	oem_img_withmagic_logo + 16,
};

/* WinCE pictures */
#ifdef CONFIG_WINCE_FEATURE
struct oem_picture oem_pic_cetext = {
	0,	0,	0xca,	0x18,	"Start WinCE",	oem_img_cetext,
//	3,	0,	0x126,	0x25,	"Start WinCE",	oem_img_sztext,
};

struct oem_picture oem_pic_celogo = {
	0,	0,	0xbd,	0xc2,	"Windows logo", oem_img_withmagic_celogo + 16,
};

struct oem_picture oem_pic_mstext = {
	0,	0,	0x8e,	0x10,	"Microsoft(R)", oem_img_mstext,
};

#if 0
struct oem_picture oem_pic_bar = {
	0,	0,	0xad,	0xe,	"Progress Bar", oem_img_bar,
};

struct oem_picture oem_pic_spark = {
	0,	0,	0x40,	0xe,	"Progress Spark", oem_img_spark,
};

struct oem_picture oem_pic_spark2 = {
	0,	0,	0x27,	0xe,	"Harf Spark", oem_img_spark2,
};

struct oem_picture oem_pic_smwd = {
	0,	0,	0x21,	0x1c,	"SM Window", oem_img_smwd,
};

struct oem_picture oem_pic_cross = {
	0,	0,	0x14,	0x14,	"Cross", oem_img_cross,
};
#endif
#endif

#ifdef CONFIG_OEM_MULTI_OS

struct oem_picture oem_pic_pungin = {
	0,	0,	0x50,	0x50,	"Linux Pungin", oem_img_pungin,
};

struct oem_picture oem_pic_window = {
	0,	0,	0x50,	0x50,	"MS Window", oem_img_window,
};
#endif
