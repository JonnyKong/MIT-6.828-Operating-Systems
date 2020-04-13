	.text

/* Switch from current_thread to next_thread. Make next_thread
 * the current_thread, and set next_thread to 0.
 * Use eax as a temporary register; it is caller saved.
 */
	.globl thread_switch
thread_switch:
	/* push current context to stack, then save esp */
	pushal
	movl current_thread, %eax
	movl %esp, (%eax)

	/* set current_thread to next_thread */
	movl next_thread, %eax
	movl %eax, current_thread
	
	/* restore next esp, then restore next context */
	movl (%eax), %esp
	popal

	/* set next_thread to zero */
	movl $0x0, next_thread
	ret				/* pop return address from stack */