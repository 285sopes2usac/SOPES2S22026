#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SOPES2");
MODULE_DESCRIPTION("Manejo de task_struct con LKM");
MODULE_VERSION("1.0");

static int target_pid = -1;
module_param(target_pid, int, 0444);

static int max_tasks = 20;
module_param(max_tasks, int, 0444);

static bool show_threads = true;
module_param(show_threads, bool, 0444);

static void print_task_info(const char *tag, struct task_struct *task)
{
	if (!task) {
		pr_info("task_lkm [%s] task=NULL\n", tag);
		return;
	}

	    pr_info("task_lkm [%s] comm=%s pid=%d tgid=%d ppid=%d prio=%d state=%u cpu=%d\n",
			tag,
			task->comm,
			task->pid,
			task->tgid,
			task_ppid_nr(task),
			task->prio,
		    (unsigned int)task->__state,
			task_cpu(task));
}

static struct task_struct *get_task_by_pid_ref(pid_t pid)
{
	struct task_struct *task = NULL;

	rcu_read_lock(); // Protección RCU para acceder a la lista de procesos
	task = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();

	return task;
}

static void list_processes(int limit)
{
	struct task_struct *task;
	int count = 0;

	if (limit <= 0)
		return;

	pr_info("task_lkm listando hasta %d procesos:\n", limit);

	rcu_read_lock();// Protección RCU para acceder a la lista de procesos
	for_each_process(task) {
		pr_info("task_lkm [list] comm=%s pid=%d tgid=%d ppid=%d state=%u\n",
				task->comm,
				task->pid,
				task->tgid,
				task_ppid_nr(task),
			(unsigned int)task->__state);

		count++;
		if (count >= limit)
			break;
	}
	rcu_read_unlock();
}

static void list_thread_group(struct task_struct *leader)
{
	struct task_struct *t;

	if (!leader)
		return;

	pr_info("task_lkm hilos del TGID=%d (leader pid=%d):\n",
			leader->tgid, leader->pid);

	rcu_read_lock(); // Protección RCU para acceder a la lista de procesos
	for_each_thread(leader, t) {
		pr_info("task_lkm [thread] comm=%s pid=%d tgid=%d state=%u\n",
			t->comm, t->pid, t->tgid, (unsigned int)t->__state);
	}
	rcu_read_unlock();
}

static void inspect_parent(struct task_struct *task)
{
	struct task_struct *parent;

	if (!task)
		return;

	rcu_read_lock();
	parent = rcu_dereference(task->real_parent); // Acceso seguro al padre con RCU
	if (parent)
		print_task_info("parent", parent);
	rcu_read_unlock();
}

static int __init task_lkm_init(void)
{
	struct task_struct *target;

	pr_info("task_lkm cargado. target_pid=%d max_tasks=%d show_threads=%d\n",
			target_pid, max_tasks, show_threads);

	print_task_info("current", current);
	inspect_parent(current);
	list_processes(max_tasks);

	if (target_pid >= 0) {
		target = get_task_by_pid_ref(target_pid);
		if (!target) {
			pr_info("task_lkm no se encontro task_struct para pid=%d\n", target_pid);
			return 0;
		}

		print_task_info("target", target);
		inspect_parent(target);
		if (show_threads)
			list_thread_group(target);

		put_task_struct(target);
	}

	return 0;
}

static void __exit task_lkm_exit(void)
{
	pr_info("task_lkm descargado\n");
}

module_init(task_lkm_init);
module_exit(task_lkm_exit);