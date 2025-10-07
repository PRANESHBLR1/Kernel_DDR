#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe8213e80, "_printk" },
	{ 0x7e2232fb, "ioread32" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x12ad300e, "iounmap" },
	{ 0xd272d446, "__fentry__" },
	{ 0x97dd6ca9, "ioremap" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xe8213e80,
	0x7e2232fb,
	0xd272d446,
	0x12ad300e,
	0xd272d446,
	0x97dd6ca9,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"_printk\0"
	"ioread32\0"
	"__x86_return_thunk\0"
	"iounmap\0"
	"__fentry__\0"
	"ioremap\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "1667F6053A332E0363E31D7");
