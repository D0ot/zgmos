.section .text.uvec
.global uvec_enter
.align 32
uvec_enter:
  csrrw a0, sscratch, a0

  # ra sp gp tp
  sd ra, (0 * 8)(a0)
  sd sp, (1 * 8)(a0)
  sd gp, (2 * 8)(a0)
  sd tp, (3 * 8)(a0)

  # t0 t1 t2
  sd t0, (4 * 8)(a0)
  sd t1, (5 * 8)(a0)
  sd t2, (6 * 8)(a0)

  # s0 s1
  sd s0, (7 * 8)(a0)
  sd s1, (8 * 8)(a0)

  # a0 ~ a7
  # sd a0, (9 * 8)(a0)
  sd a1, (10* 8)(a0)
  sd a2, (11* 8)(a0)
  sd a3, (12* 8)(a0)
  sd a4, (13* 8)(a0)
  sd a5, (14* 8)(a0)
  sd a6, (15* 8)(a0)
  sd a7, (16* 8)(a0)

  # s2 ~ s11
  sd s0, (17* 8)(a0)
  sd s2, (18* 8)(a0)
  sd s3, (19* 8)(a0)
  sd s4, (20* 8)(a0)
  sd s5, (21* 8)(a0)
  sd s6, (22* 8)(a0)
  sd s7, (23* 8)(a0)
  sd s8, (24* 8)(a0)
  sd s9, (25* 8)(a0)
  sd s10,(26* 8)(a0)
  sd s11,(27* 8)(a0)

  # t3 ~ t6
  sd t3, (28* 8)(a0)
  sd t4, (29* 8)(a0)
  sd t5, (30* 8)(a0)
  sd t6, (31* 8)(a0)

# epc         32 *8
# kernel_satp 33 *8
# kernel_sp   34 *8
# hartid      35 *8
# utrap_entry 36 *8

  # get real a0
  csrr t0, sscratch
  
  # save a0 to right position
  sd t0, (9 * 8)(a0)

  ld sp, (34* 8)(a0)

  ld tp, (35* 8)(a0)

  ld t0, (33* 8)(a0)

  ld t1, (36* 8)(a0)

  csrw satp, t0

  sfence.vma

  # jump to utrap_entry
  jr t1



.global uvec_ret
# a0: trapframe va, should saved in sscratch
# a1: the user satp value
uvec_ret:

  csrw satp, a1

  sfence.vma

  ld t0, (9 * 8)(a0)
  
  # save previously saved a0 to sscratch
  csrw sscratch, t0

  # ra sp gp tp
  ld ra, (0 * 8)(a0)
  ld sp, (1 * 8)(a0)
  ld gp, (2 * 8)(a0)
  ld tp, (3 * 8)(a0)

  # t0 t1 t2
  ld t0, (4 * 8)(a0)
  ld t1, (5 * 8)(a0)
  ld t2, (6 * 8)(a0)

  # s0 s1
  ld s0, (7 * 8)(a0)
  ld s1, (8 * 8)(a0)

  # a0 ~ a7
  # ld a0, (9 * 8)(a0)
  ld a1, (10* 8)(a0)
  ld a2, (11* 8)(a0)
  ld a3, (12* 8)(a0)
  ld a4, (13* 8)(a0)
  ld a5, (14* 8)(a0)
  ld a6, (15* 8)(a0)
  ld a7, (16* 8)(a0)

  # s2 ~ s11
  ld s0, (17* 8)(a0)
  ld s2, (18* 8)(a0)
  ld s3, (19* 8)(a0)
  ld s4, (20* 8)(a0)
  ld s5, (21* 8)(a0)
  ld s6, (22* 8)(a0)
  ld s7, (23* 8)(a0)
  ld s8, (24* 8)(a0)
  ld s9, (25* 8)(a0)
  ld s10,(26* 8)(a0)
  ld s11,(27* 8)(a0)

  # t3 ~ t6
  ld t3, (28* 8)(a0)
  ld t4, (29* 8)(a0)
  ld t5, (30* 8)(a0)
  ld t6, (31* 8)(a0)

  csrrw a0, sscratch, a0

  sret
