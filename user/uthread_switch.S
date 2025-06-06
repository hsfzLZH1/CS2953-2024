	.text

	/*
         * save the old thread's registers,
         * restore the new thread's registers.
         */

/*
thread_switch(struct thread*old,struct thread*new):
switch the current thread from old to new
store the current register in old thread stack,
load new register values from new thread stack.
*/
	.globl thread_switch
thread_switch:
	/* YOUR CODE HERE */
  sd ra, 0(a0)
  sd sp, 8(a0)
  sd gp, 16(a0)
  sd tp, 24(a0)
  sd t0, 32(a0)
  sd t1, 40(a0)
  sd t2, 48(a0)
  sd s0, 56(a0)
  sd s1, 64(a0)
  sd a0, 72(a0)
  sd a1, 80(a0)
  sd a2, 88(a0)
  sd a3, 96(a0)
  sd a4, 104(a0)
  sd a5, 112(a0)
  sd a6, 120(a0)
  sd a7, 128(a0)
  sd s2, 136(a0)
  sd s3, 144(a0)
  sd s4, 152(a0)
  sd s5, 160(a0)
  sd s6, 168(a0)
  sd s7, 176(a0)
  sd s8, 184(a0)
  sd s9, 192(a0)
  sd s10, 200(a0)
  sd s11, 208(a0)
  sd t3, 216(a0)
  sd t4, 224(a0)
  sd t5, 232(a0)
  sd t6, 240(a0)

  ld ra, 0(a1)
  ld sp, 8(a1)
  ld gp, 16(a1)
  ld tp, 24(a1)
  ld t0, 32(a1)
  ld t1, 40(a1)
  ld t2, 48(a1)
  ld s0, 56(a1)
  ld s1, 64(a1)
  ld a0, 72(a1)
  ld a2, 88(a1)
  ld a3, 96(a1)
  ld a4, 104(a1)
  ld a5, 112(a1)
  ld a6, 120(a1)
  ld a7, 128(a1)
  ld s2, 136(a1)
  ld s3, 144(a1)
  ld s4, 152(a1)
  ld s5, 160(a1)
  ld s6, 168(a1)
  ld s7, 176(a1)
  ld s8, 184(a1)
  ld s9, 192(a1)
  ld s10, 200(a1)
  ld s11, 208(a1)
  ld t3, 216(a1)
  ld t4, 224(a1)
  ld t5, 232(a1)
  ld t6, 240(a1)

  ld a1, 80(a1) /* restore a1 at last */

	ret    /* return to ra */
