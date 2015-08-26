# on revision 1d4aa8bbf97ab1d725cf84328ef463dce29633b7 #

```
[ 1166.573372] ERROR: AddressSanitizer: heap-use-after-free on address ffff880032afae44
[ 1166.575026] Accessed by thread T24895:
[ 1166.575865]   #0 ffffffff810dd483 (asan_report_error+0x163/0x3b0)
[ 1166.577165]   #1 ffffffff810dc6c0 (asan_check_region+0x30/0x40)
[ 1166.578443]   #2 ffffffff810dd713 (__tsan_read1+0x13/0x20)
[ 1166.579629]   #3 ffffffff814d4edf (kobject_put+0x1f/0x80)
[ 1166.580766]   #4 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.581923]   #5 ffffffff8161c80b (scsi_host_dev_release+0x13b/0x160)
[ 1166.583272]   #6 ffffffff815f8573 (device_release+0x53/0x100)
[ 1166.584464]   #7 ffffffff814d50df (kobject_release+0x6f/0xc0)
[ 1166.585660]   #8 ffffffff814d4efa (kobject_put+0x3a/0x80)
[ 1166.586836]   #9 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.587955]   #10 ffffffff81629b04 (scsi_target_dev_release+0x34/0x40)
[ 1166.589395]   #11 ffffffff815f8573 (device_release+0x53/0x100)
[ 1166.590672]   #12 ffffffff814d50df (kobject_release+0x6f/0xc0)
[ 1166.591912]   #13 ffffffff814d4efa (kobject_put+0x3a/0x80)
[ 1166.592668]   #14 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.593153]   #15 ffffffff8162e5ca (scsi_device_dev_release_usercontext+0x35a/0x370)
[ 1166.593794]   #16 ffffffff81110bd9 (execute_in_process_context+0xc9/0xe0)
[ 1166.594384]   #17 ffffffff8162e229 (scsi_device_dev_release+0x29/0x40)
[ 1166.594929]   #18 ffffffff815f8573 (device_release+0x53/0x100)
[ 1166.595440]   #19 ffffffff814d50df (kobject_release+0x6f/0xc0)
[ 1166.595930]   #20 ffffffff814d4efa (kobject_put+0x3a/0x80)
[ 1166.596414]   #21 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.596870]   #22 ffffffff81619472 (scsi_device_put+0x62/0x80)
[ 1166.597389]   #23 ffffffff81679a4b (sg_remove_sfp_usercontext+0xdb/0x160)
[ 1166.597955]   #24 ffffffff81111680 (process_one_work+0x2d0/0x750)
[ 1166.598488]   #25 ffffffff81111d63 (worker_thread+0x263/0x640)
[ 1166.598977]   #26 ffffffff8111c0d2 (kthread+0x132/0x140)
[ 1166.599457]   #27 ffffffff819257dc (ret_from_fork+0x7c/0xb0)
[ 1166.599923] 
[ 1166.600083] Freed by thread T4068:
[ 1166.600527]   #0 ffffffff810dc867 (asan_slab_free+0x77/0xb0)
[ 1166.601571]   #1 ffffffff81280b7a (kfree+0x9a/0x240)
[ 1166.602637]   #2 ffffffff812d27f0 (setxattr+0x170/0x1b0)
[ 1166.603381]   #3 ffffffff812d2c50 (SyS_lsetxattr+0xc0/0x100)
[ 1166.604187]   #4 ffffffff81925882 (system_call_fastpath+0x16/0x1b)
[ 1166.604995] 
[ 1166.605232] Allocated by thread T4068:
[ 1166.605741]   #0 ffffffff810dc78a (asan_slab_alloc+0x4a/0xb0)
[ 1166.606524]   #1 ffffffff8128251c (__kmalloc+0xbc/0x500)
[ 1166.607255]   #2 ffffffff812d2724 (setxattr+0xa4/0x1b0)
[ 1166.607965]   #3 ffffffff812d2c50 (SyS_lsetxattr+0xc0/0x100)
[ 1166.608741]   #4 ffffffff81925882 (system_call_fastpath+0x16/0x1b)
[ 1166.609571] 
[ 1166.609784] Shadow bytes around the buggy address:
[ 1166.610489]   ffff880032afab80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.611535]   ffff880032afac00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.612566]   ffff880032afac80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.613592]   ffff880032afad00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.614628]   ffff880032afad80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.615659] =>ffff880032afae00: fd fd fd fd fd fd fd fd[fd]fd fd fd fd fd fd fd
[ 1166.616687]   ffff880032afae80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.617717]   ffff880032afaf00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.619142]   ffff880032afaf80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.620639]   ffff880032afb000: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.622122]   ffff880032afb080: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.623124] Shadow byte legend (one shadow byte represents 8 application bytes):
[ 1166.624554]   Addressable:           00
[ 1166.625322]   Partially addressable: 01 02 03 04 05 06 07 
[ 1166.626401]   Heap redzone:          fa
[ 1166.627151]   Heap kmalloc redzone:  fb
[ 1166.627884]   Freed heap region:     fd
```

```
[ 1166.631626] ERROR: AddressSanitizer: heap-use-after-free on address ffff880032afae08
[ 1166.633092] Accessed by thread T24895:
[ 1166.633837]   #0 ffffffff810dd483 (asan_report_error+0x163/0x3b0)
[ 1166.635014]   #1 ffffffff810dc6c0 (asan_check_region+0x30/0x40)
[ 1166.636149]   #2 ffffffff810dd773 (__tsan_read8+0x13/0x20)
[ 1166.637218]   #3 ffffffff814d4f10 (kobject_put+0x50/0x80)
[ 1166.638273]   #4 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.639314]   #5 ffffffff8161c80b (scsi_host_dev_release+0x13b/0x160)
[ 1166.640555]   #6 ffffffff815f8573 (device_release+0x53/0x100)
[ 1166.641685]   #7 ffffffff814d50df (kobject_release+0x6f/0xc0)
[ 1166.642794]   #8 ffffffff814d4efa (kobject_put+0x3a/0x80)
[ 1166.643840]   #9 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.644883]   #10 ffffffff81629b04 (scsi_target_dev_release+0x34/0x40)
[ 1166.646143]   #11 ffffffff815f8573 (device_release+0x53/0x100)
[ 1166.647267]   #12 ffffffff814d50df (kobject_release+0x6f/0xc0)
[ 1166.648399]   #13 ffffffff814d4efa (kobject_put+0x3a/0x80)
[ 1166.649458]   #14 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.650512]   #15 ffffffff8162e5ca (scsi_device_dev_release_usercontext+0x35a/0x370)
[ 1166.651785]   #16 ffffffff81110bd9 (execute_in_process_context+0xc9/0xe0)
[ 1166.652385]   #17 ffffffff8162e229 (scsi_device_dev_release+0x29/0x40)
[ 1166.652943]   #18 ffffffff815f8573 (device_release+0x53/0x100)
[ 1166.653458]   #19 ffffffff814d50df (kobject_release+0x6f/0xc0)
[ 1166.653949]   #20 ffffffff814d4efa (kobject_put+0x3a/0x80)
[ 1166.654753]   #21 ffffffff815f8b44 (put_device+0x24/0x30)
[ 1166.655540]   #22 ffffffff81619472 (scsi_device_put+0x62/0x80)
[ 1166.656362]   #23 ffffffff81679a4b (sg_remove_sfp_usercontext+0xdb/0x160)
[ 1166.656945]   #24 ffffffff81111680 (process_one_work+0x2d0/0x750)
[ 1166.657529]   #25 ffffffff81111d63 (worker_thread+0x263/0x640)
[ 1166.658042]   #26 ffffffff8111c0d2 (kthread+0x132/0x140)
[ 1166.658495]   #27 ffffffff819257dc (ret_from_fork+0x7c/0xb0)
[ 1166.658960] 
[ 1166.659133] Freed by thread T4068:
[ 1166.659434]   #0 ffffffff810dc867 (asan_slab_free+0x77/0xb0)
[ 1166.659912]   #1 ffffffff81280b7a (kfree+0x9a/0x240)
[ 1166.660360]   #2 ffffffff812d27f0 (setxattr+0x170/0x1b0)
[ 1166.660809]   #3 ffffffff812d2c50 (SyS_lsetxattr+0xc0/0x100)
[ 1166.661762]   #4 ffffffff81925882 (system_call_fastpath+0x16/0x1b)
[ 1166.662975] 
[ 1166.663296] Allocated by thread T4068:
[ 1166.664064]   #0 ffffffff810dc78a (asan_slab_alloc+0x4a/0xb0)
[ 1166.665346]   #1 ffffffff8128251c (__kmalloc+0xbc/0x500)
[ 1166.666989]   #2 ffffffff812d2724 (setxattr+0xa4/0x1b0)
[ 1166.668350]   #3 ffffffff812d2c50 (SyS_lsetxattr+0xc0/0x100)
[ 1166.669480]   #4 ffffffff81925882 (system_call_fastpath+0x16/0x1b)
[ 1166.670704] 
[ 1166.671214] Shadow bytes around the buggy address:
[ 1166.672748]   ffff880032afab80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.675045]   ffff880032afac00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.677325]   ffff880032afac80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.679598]   ffff880032afad00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.681871]   ffff880032afad80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.684141] =>ffff880032afae00: fd[fd]fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.686402]   ffff880032afae80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.688671]   ffff880032afaf00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.690937]   ffff880032afaf80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.693229]   ffff880032afb000: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.695518]   ffff880032afb080: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
[ 1166.697732] Shadow byte legend (one shadow byte represents 8 application bytes):
[ 1166.699966]   Addressable:           00
[ 1166.701164]   Partially addressable: 01 02 03 04 05 06 07 
[ 1166.702875]   Heap redzone:          fa
[ 1166.704097]   Heap kmalloc redzone:  fb
[ 1166.705277]   Freed heap region:     fd
```

# On revision d8efd82eece89f8a5790b0febf17522affe9e1f1 #

```
ERROR: AddressSanitizer: heap-use-after-free on address ffff88005a8ccb08
ffff88005a8ccb08 is located 8 bytes inside of 576-byte region [ffff88005a8ccb00, ffff88005a8ccd40)
WRITE of size 8 at ffff88005a8ccb08 by thread T4740:
  #0      inlined     (asan_report_error+0x367/0x480) asan_describe_heap_address ./arch/x86/mm/asan/report.c:175
  #0 ffffffff810d9917 (asan_report_error+0x367/0x480) ./arch/x86/mm/asan/report.c:292
  #1 ffffffff810d8bd2 (asan_check_region.part.1+0x192/0x200) ./arch/x86/mm/asan/asan.c:244
  #2 ffffffff810d8de3 (__tsan_write8+0x23/0x30) ./arch/x86/mm/asan/asan.c:262
  #3      inlined     (aio_migratepage+0xd1/0x140) spin_unlock_irqrestore ./include/linux/spinlock.h:348
  #3 ffffffff812f7d71 (aio_migratepage+0xd1/0x140) ./fs/aio.c:230
  #4 ffffffff812761b6 (move_to_new_page+0xd6/0x2f0) ./mm/migrate.c:702
  #5      inlined     (migrate_pages+0x81a/0x920) __unmap_and_move ./mm/migrate.c:844
  #5      inlined     (migrate_pages+0x81a/0x920) unmap_and_move ./mm/migrate.c:885
  #5 ffffffff81276d9a (migrate_pages+0x81a/0x920) ./mm/migrate.c:1048
  #6      inlined     (compact_zone+0x4cb/0x700) update_nr_listpages ./mm/compaction.c:772
  #6 ffffffff8123915b (compact_zone+0x4cb/0x700) ./mm/compaction.c:986
  #7 ffffffff8123949a (compact_zone_order+0x10a/0x160)  ./mm/compaction.c:1032
  #8 ffffffff8123999b (try_to_compact_pages+0xeb/0x130) ./mm/compaction.c:1082
  #9 ffffffff818cdcfb (__alloc_pages_direct_compact+0x129/0x2d0) ./mm/page_alloc.c:2234
  #10      inlined     (__alloc_pages_nodemask+0x91d/0xdc0) __alloc_pages_slowpath ./mm/page_alloc.c:2568
  #10 ffffffff8121285d (__alloc_pages_nodemask+0x91d/0xdc0) ./mm/page_alloc.c:2731
  #11 ffffffff8126fc7f (kmem_getpages+0x5f/0x1d0) ./mm/slab.c:1759
  #12 ffffffff81270897 (fallback_alloc+0x177/0x250) ././arch/x86/include/asm/irqflags.h:39
  #13 ffffffff812706eb (____cache_alloc_node+0x11b/0x150) ./mm/slab.c:3352
  #14      inlined     (__kmalloc+0x367/0x500) __do_cache_alloc ./mm/slab.c:3430
  #14      inlined     (__kmalloc+0x367/0x500) slab_alloc ./mm/slab.c:3462
  #14      inlined     (__kmalloc+0x367/0x500) __do_kmalloc ./mm/slab.c:3748
  #14 ffffffff81272a17 (__kmalloc+0x367/0x500) ./mm/slab.c:3762
  #15 ffffffff812be7c4 (seq_read+0x254/0x7b0) ./fs/seq_file.c:235
  #16 ffffffff8128bdea (vfs_read+0xfa/0x240) ??:0
  #17 ffffffff8128d062 (SyS_read+0x72/0xd0) ??:0
  #18 ffffffff818ea235 (sysenter_dispatch+0x7/0x1a) ./arch/x86/ia32/ia32entry.S:164

freed by thread T2002 here:
  #0      inlined     (kmem_cache_free+0x55/0x2e0) __cache_free ./mm/slab.c:3590
  #0 ffffffff81270a35 (kmem_cache_free+0x55/0x2e0) ./mm/slab.c:3799
  #1 ffffffff812f6d31 (free_ioctx_rcu+0x41/0x50) ./fs/aio.c:398
  #2      inlined     (rcu_process_callbacks+0x2a2/0x890) rcu_do_batch ./kernel/rcutree.c:1992
  #2      inlined     (rcu_process_callbacks+0x2a2/0x890) invoke_rcu_callbacks ./kernel/rcutree.c:2236
  #2      inlined     (rcu_process_callbacks+0x2a2/0x890) __rcu_process_callbacks ./kernel/rcutree.c:2206
  #2 ffffffff811afff2 (rcu_process_callbacks+0x2a2/0x890) ./kernel/rcutree.c:2220
  #3      inlined     (__do_softirq+0x172/0x380) trace_softirq_exit ./kernel/softirq.c:251
  #3 ffffffff810ea742 (__do_softirq+0x172/0x380) ./kernel/softirq.c:252
  #4      inlined     (irq_exit+0x10d/0x120) invoke_softirq ./kernel/softirq.c:332
  #4 ffffffff810eab8d (irq_exit+0x10d/0x120) ./kernel/softirq.c:365
  #5 ffffffff818ea90e (smp_apic_timer_interrupt+0x5e/0x70) ./arch/x86/kernel/apic/apic.c:933
  #6 ffffffff818e96ca (apic_timer_interrupt+0x6a/0x70) ./arch/x86/kernel/entry_64.S:1181
  #7 ffffffff812d3e0c (free_buffer_head+0x3c/0x70) ./fs/buffer.c:3341
  #8 ffffffff812d4008 (try_to_free_buffers+0xb8/0x110) ./fs/buffer.c:3256
  #9 ffffffff8140e28b (jbd2_journal_try_to_free_buffers+0xfb/0x190) ./fs/jbd2/transaction.c:1907
  #10 ffffffff8138cb3e (ext4_releasepage+0x8e/0x110) ./fs/ext4/inode.c:3005
  #11 ffffffff81206008 (try_to_release_page+0x68/0x90) ./mm/filemap.c:2603
  #12 ffffffff812d5b02 (block_invalidatepage+0x142/0x170) ./fs/buffer.c:1549
  #13 ffffffff8138d8d4 (ext4_invalidatepage+0x84/0xf0) ./fs/ext4/inode.c:2965
  #14 ffffffff8138e2e6 (ext4_da_invalidatepage+0x56/0x500) ./fs/ext4/inode.c:2811
  #15 ffffffff8121a9e8 (truncate_inode_page+0xc8/0xd0) ??:0

previously allocated by thread T5840 here:
  #0      inlined     (kmem_cache_alloc+0x9a/0x4c0) slab_alloc ./mm/slab.c:3474
  #0 ffffffff81272c4a (kmem_cache_alloc+0x9a/0x4c0) ./mm/slab.c:3629
  #1      inlined     (SyS_io_setup+0x10c/0xce0) ioctx_alloc ./fs/aio.c:563
  #1      inlined     (SyS_io_setup+0x10c/0xce0) SYSC_io_setup ./fs/aio.c:1100
  #1 ffffffff812f7fec (SyS_io_setup+0x10c/0xce0) ./fs/aio.c:1083
  #2 ffffffff818e8b02 (system_call_fastpath+0x16/0x1b) ./arch/x86/kernel/entry_64.S:645

Shadow bytes around the buggy address:
  ffff88005a8cc880: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  ffff88005a8cc900: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  ffff88005a8cc980: 00 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa
  ffff88005a8cca00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff88005a8cca80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>ffff88005a8ccb00: fd[fd]fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88005a8ccb80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88005a8ccc00: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88005a8ccc80: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88005a8ccd00: fd fd fd fd fd fd fd fd fa fa fa fa fa fa fa fa
  ffff88005a8ccd80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap redzone:          fa
  Heap kmalloc redzone:  fb
  Freed heap region:     fd
  Shadow gap:            fe
```

```
ERROR: AddressSanitizer: heap-buffer-overflow on address ffff880031f23b34
ffff880031f23b34 is located 0 bytes to the right of 52-byte region [ffff880031f23b00, ffff880031f23b34)
READ of size 8 at ffff880031f23b34 by thread T1168:
  #0      inlined     (asan_report_error+0x3e7/0x500) asan_describe_heap_address ./arch/x86/mm/asan/report.c:191
  #0 ffffffff810d9fc7 (asan_report_error+0x3e7/0x500) ./arch/x86/mm/asan/report.c:309
  #1 ffffffff810d8a44 (asan_check_region.part.1+0x144/0x2a0) ./arch/x86/mm/asan/asan.c:263
  #2      inlined     (__tsan_read8+0x20/0x30) asan_check_region ./arch/x86/mm/asan/asan.c:276
  #2 ffffffff810d8c50 (__tsan_read8+0x20/0x30) ./arch/x86/mm/asan/asan.c:276
  #3 ffffffff812b180f (__d_lookup_rcu+0x19f/0x250) ./fs/dcache.c:215
  #4 ffffffff8129c597 (lookup_fast+0x67/0x510) ./fs/namei.c:1369
  #5      inlined     (path_lookupat+0x1ef/0xed0) walk_component ./fs/namei.c:1540
  #5      inlined     (path_lookupat+0x1ef/0xed0) lookup_last ./fs/namei.c:1938
  #5 ffffffff8129ef0f (path_lookupat+0x1ef/0xed0) ./fs/namei.c:1972
  #6 ffffffff8129fc2b (filename_lookup+0x3b/0x110) ./fs/namei.c:2011
  #7 ffffffff812a5af4 (user_path_at_empty+0x54/0xa0) ./fs/namei.c:2159
  #8 ffffffff812a5b51 (user_path_at+0x11/0x20) ./fs/namei.c:2170
  #9 ffffffff81293b82 (vfs_fstatat+0x52/0xa0) ??:0
  #10 ffffffff81293c0e (vfs_lstat+0x1e/0x20) ??:0
  #11 ffffffff810dadaa (sys32_lstat64+0x1a/0x40) ??:0
  #12 ffffffff8188fb89 (ia32_sysret+0x0/0x5) ./arch/x86/ia32/ia32entry.S:428

allocated by thread T1136 here:
  #0      inlined     (__kmalloc+0xbc/0x500) slab_alloc ./mm/slab.c:3471
  #0      inlined     (__kmalloc+0xbc/0x500) __do_kmalloc ./mm/slab.c:3748
  #0 ffffffff81272e7c (__kmalloc+0xbc/0x500) ./mm/slab.c:3762
  #1      inlined     (__d_alloc+0x295/0x2e0) kmalloc ./include/linux/slab.h:413
  #1 ffffffff812b1185 (__d_alloc+0x295/0x2e0) ./fs/dcache.c:1574
  #2 ffffffff812b11fd (d_alloc+0x2d/0xf0) ./fs/dcache.c:1624
  #3 ffffffff8129bf2b (lookup_dcache+0xbb/0x100) ./fs/namei.c:1301
  #4 ffffffff8129bf91 (__lookup_hash+0x21/0x70) ./fs/namei.c:1341
  #5      inlined     (SYSC_renameat+0x492/0x580) lookup_hash ./fs/namei.c:2094
  #5 ffffffff812a2552 (SYSC_renameat+0x492/0x580) ./fs/namei.c:4159
  #6 ffffffff812a665e (SyS_renameat+0xe/0x10) ./fs/namei.c:4091
  #7      inlined     (SyS_rename+0x1b/0x20) SYSC_rename ./fs/namei.c:4200
  #7 ffffffff812a667b (SyS_rename+0x1b/0x20) ./fs/namei.c:4198
  #8 ffffffff8188f775 (sysenter_dispatch+0x7/0x1a) ./arch/x86/ia32/ia32entry.S:163

Shadow bytes around the buggy address:
  ffff880031f23880: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff880031f23900: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff880031f23980: fd fd fd fd fd fd fd fd fa fa fa fa fa fa fa fa
  ffff880031f23a00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff880031f23a80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>ffff880031f23b00: 00 00 00 00 00 00[04]fb fa fa fa fa fa fa fa fa
  ffff880031f23b80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff880031f23c00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff880031f23c80: fd fd fd fd fd fd fd fd fa fa fa fa fa fa fa fa
  ffff880031f23d00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff880031f23d80: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
```

# On revision 2457aaf73a97a97c8596ed3903bd09601976f3bc #

```
ERROR: AddressSanitizer: heap-use-after-free on address ffff8800244d94c9
ffff8800244d94c9 is located 73 bytes inside of 216-byte region [ffff8800244d9480, ffff8800244d9558)
READ of size 8 at ffff8800244d94c9 by thread T6094:
  #0      inlined     (asan_report_error+0x3e6/0x500) asan_describe_heap_address ./arch/x86/mm/asan/report.c:191
  #0 ffffffff810d9fd6 (asan_report_error+0x3e6/0x500) ./arch/x86/mm/asan/report.c:309
  #1 ffffffff810d8a54 (asan_check_region.part.1+0x144/0x2a0) ./arch/x86/mm/asan/asan.c:263
  #2      inlined     (__tsan_read8+0x20/0x30) asan_check_region ./arch/x86/mm/asan/asan.c:276
  #2 ffffffff810d8c60 (__tsan_read8+0x20/0x30) ./arch/x86/mm/asan/asan.c:276
  #3 ffffffff81250708 (mm_find_pmd+0x38/0xd0) ./mm/rmap.c:571
  #4 ffffffff812507d9 (__page_check_address+0x39/0x160) ./mm/rmap.c:608
  #5      inlined     (try_to_unmap_one+0x4f/0x520) page_check_address ./include/linux/rmap.h:207
  #5 ffffffff8125169f (try_to_unmap_one+0x4f/0x520) ./mm/rmap.c:1191
  #6 ffffffff81251cb4 (try_to_unmap_file+0x144/0x8d0) ./mm/rmap.c:1528
  #7 ffffffff81253799 (try_to_munlock+0x29/0x40) ./mm/rmap.c:1663
  #8 ffffffff812462d9 (__munlock_isolated_page+0x29/0x50) mlock.c:0
  #9 ffffffff81246640 (__munlock_pagevec+0x2e0/0x4c0) mlock.c:0
  #10 ffffffff81246f1f (munlock_vma_pages_range+0x38f/0x3e0) ??:0
  #11 ffffffff8124d3d0 (exit_mmap+0x90/0x1e0) ./mm/internal.h:168
  #12 ffffffff810de7cf (mmput+0x5f/0x150) ./kernel/fork.c:612
  #13      inlined     (do_exit+0x3e4/0x11b0) exit_mm ./kernel/exit.c:491
  #13 ffffffff810e6584 (do_exit+0x3e4/0x11b0) ./kernel/exit.c:776
  #14 ffffffff810e740b (do_group_exit+0x7b/0x130) ./kernel/exit.c:920
  #15 ffffffff810fd5c1 (get_signal_to_deliver+0x281/0x870) ./kernel/signal.c:2370
  #16 ffffffff8107d064 (do_signal+0x54/0xb40) signal.c:0
  #17 ffffffff8107dbcd (do_notify_resume+0x7d/0xa0) ??:0
  #18 ffffffff8188e91a (int_signal+0x12/0x17) ./arch/x86/kernel/entry_64.S:806

freed by thread T3 here:
  #0      inlined     (kmem_cache_free+0x55/0x2e0) __cache_free ./mm/slab.c:3590
  #0 ffffffff812704f5 (kmem_cache_free+0x55/0x2e0) ./mm/slab.c:3799
  #1 ffffffff8128eb64 (file_free_rcu+0x44/0x50) ./fs/file_table.c:51
  #2      inlined     (rcu_process_callbacks+0x2a2/0x8a0) __rcu_reclaim ./kernel/rcu.h:111
  #2      inlined     (rcu_process_callbacks+0x2a2/0x8a0) rcu_do_batch ./kernel/rcutree.c:1988
  #2      inlined     (rcu_process_callbacks+0x2a2/0x8a0) invoke_rcu_callbacks ./kernel/rcutree.c:2236
  #2      inlined     (rcu_process_callbacks+0x2a2/0x8a0) __rcu_process_callbacks ./kernel/rcutree.c:2206
  #2 ffffffff811b0702 (rcu_process_callbacks+0x2a2/0x8a0) ./kernel/rcutree.c:2220
  #3 ffffffff810eaea2 (__do_softirq+0x172/0x380) ./kernel/softirq.c:251
  #4 ffffffff810eb0d8 (run_ksoftirqd+0x28/0x40) ./kernel/softirq.c:765
  #5 ffffffff81122637 (smpboot_thread_fn+0x187/0x260) ./kernel/smpboot.c:160
  #6 ffffffff81115e36 (kthread+0x126/0x130) kthread.c:0
  #7 ffffffff8188e5ec (ret_from_fork+0x7c/0xb0) ./arch/x86/kernel/entry_64.S:554

previously allocated by thread T12678 here:
  #0      inlined     (kmem_cache_alloc+0x9a/0x4c0) slab_alloc ./mm/slab.c:3471
  #0 ffffffff812722aa (kmem_cache_alloc+0x9a/0x4c0) ./mm/slab.c:3629
  #1      inlined     (get_empty_filp+0x8f/0x2a0) kmem_cache_zalloc ./include/linux/slab.h:635
  #1 ffffffff8128f01f (get_empty_filp+0x8f/0x2a0) ./fs/file_table.c:125
  #2 ffffffff812a4c76 (path_openat+0x56/0xa90) ./fs/namei.c:3189
  #3 ffffffff812a6159 (do_filp_open+0x49/0xa0) ./fs/namei.c:3258
  #4 ffffffff8128b433 (do_sys_open+0x1a3/0x2c0) ??:0
  #5 ffffffff8130537b (compat_SyS_open+0x1b/0x20) ??:0
  #6 ffffffff8188fdb5 (sysenter_dispatch+0x7/0x1a) ./arch/x86/ia32/ia32entry.S:163

Shadow bytes around the buggy address:
  ffff8800244d9200: fa fa fa fa fa fa fa fa 00 00 00 00 00 00 00 00
  ffff8800244d9280: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  ffff8800244d9300: 00 00 00 fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff8800244d9380: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff8800244d9400: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>ffff8800244d9480: fd fd fd fd fd fd fd fd fd[fd]fd fd fd fd fd fd
  ffff8800244d9500: fd fd fd fd fd fd fd fd fd fd fd fa fa fa fa fa
  ffff8800244d9580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff8800244d9600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff8800244d9680: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
  ffff8800244d9700: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap redzone:          fa
  Heap kmalloc redzone:  fb
  Freed heap region:     fd
  Shadow gap:            fe
```

```
ERROR: AddressSanitizer: heap-use-after-free on address ffff88002994a678
ffff88002994a678 is located 56 bytes inside of 4096-byte region [ffff88002994a640, ffff88002994b640)
READ of size 8 at ffff88002994a678 by thread T478:
  #0      inlined     (asan_report_error+0x406/0x530) asan_describe_heap_address ./arch/x86/mm/asan/report.c:191
  #0 ffffffff810d9b26 (asan_report_error+0x406/0x530) ./arch/x86/mm/asan/report.c:309
  #1 ffffffff810d8bf2 (asan_check_region.part.1+0x192/0x200) ./arch/x86/mm/asan/asan.c:263
  #2      inlined     (__tsan_read8+0x28/0x30) asan_check_region ./arch/x86/mm/asan/asan.c:276
  #2 ffffffff810d8d18 (__tsan_read8+0x28/0x30) ./arch/x86/mm/asan/asan.c:276
  #3      inlined     (page_evictable+0x25/0x60) mapping_unevictable ././arch/x86/include/asm/bitops.h:329
  #3 ffffffff8121fd95 (page_evictable+0x25/0x60) ./mm/vmscan.c:3681
  #4 ffffffff812200cc (shrink_page_list+0x24c/0xf90) ./mm/vmscan.c:812
  #5 ffffffff812217aa (shrink_inactive_list+0x22a/0x7a0) ./mm/vmscan.c:1475
  #6      inlined     (shrink_lruvec+0x539/0xaa0) shrink_list ./mm/vmscan.c:1807
  #6 ffffffff81222879 (shrink_lruvec+0x539/0xaa0) ./mm/vmscan.c:2031
  #7 ffffffff81222f3e (__shrink_zone+0x15e/0x360) ./mm/vmscan.c:2204
  #8 ffffffff812231d5 (shrink_zone+0x95/0xa0) ./mm/vmscan.c:2257
  #9      inlined     (balance_pgdat+0x577/0x8f0) kswapd_shrink_zone ./mm/vmscan.c:2901
  #9 ffffffff81224ca7 (balance_pgdat+0x577/0x8f0) ./mm/vmscan.c:3075
  #10 ffffffff812252bb (kswapd+0x29b/0x630) ./mm/vmscan.c:3283
  #11 ffffffff81115996 (kthread+0x126/0x130) kthread.c:0
  #12 ffffffff818e929c (ret_from_fork+0x7c/0xb0) ./arch/x86/kernel/entry_64.S:569

freed by thread T7697 here:
  #0      inlined     (kmem_cache_free+0x55/0x2e0) __cache_free ./mm/slab.c:3590
  #0 ffffffff81270095 (kmem_cache_free+0x55/0x2e0) ./mm/slab.c:3799
  #1 ffffffff8129d469 (final_putname+0x39/0x80) ./fs/namei.c:126
  #2 ffffffff8129d748 (putname+0x48/0x60) ./fs/namei.c:219
  #3 ffffffff8128b060 (do_sys_open+0x230/0x2c0) ??:0
  #4 ffffffff8130249b (compat_SyS_open+0x1b/0x20) ??:0
  #5 ffffffff818eaa75 (sysenter_dispatch+0x7/0x1a) ./arch/x86/ia32/ia32entry.S:163

previously allocated by thread T7697 here:
  #0      inlined     (kmem_cache_alloc+0x9a/0x4c0) slab_alloc ./mm/slab.c:3471
  #0 ffffffff81271e4a (kmem_cache_alloc+0x9a/0x4c0) ./mm/slab.c:3629
  #1 ffffffff8129d51f (getname_flags+0x6f/0x230) ./fs/namei.c:144
  #2 ffffffff8129d6f2 (getname+0x12/0x20) ./fs/namei.c:210
  #3 ffffffff8128afa3 (do_sys_open+0x173/0x2c0) ??:0
  #4 ffffffff8130249b (compat_SyS_open+0x1b/0x20) ??:0
  #5 ffffffff818eaa75 (sysenter_dispatch+0x7/0x1a) ./arch/x86/ia32/ia32entry.S:163

Shadow bytes around the buggy address:
  ffff88002994a380: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff88002994a400: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff88002994a480: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff88002994a500: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  ffff88002994a580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>ffff88002994a600: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd[fd]
  ffff88002994a680: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88002994a700: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88002994a780: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88002994a800: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  ffff88002994a880: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap redzone:          fa
  Heap kmalloc redzone:  fb
  Freed heap region:     fd
  Shadow gap:            fe
```

```
AddressSanitizer: heap-buffer-overflow on address ffff88000192c000
Read of size 1 by thread T1045:
 [<ffffffff814d6ee1>] crc16+0x21/0x50 ??:0
 [<ffffffff813b9434>] ext4_group_desc_csum+0x244/0x260 ./fs/ext4/super.c:2025
 [<ffffffff813c747c>] ext4_group_desc_csum_set+0x5c/0x80 ./fs/ext4/super.c:2049
 [<ffffffff813e9e7d>] ext4_mb_mark_diskspace_used+0x37d/0x7f0 ./fs/ext4/mballoc.c:2926
 [<ffffffff813ec459>] ext4_mb_new_blocks+0x529/0x9b0 ./fs/ext4/mballoc.c:4443
 [<ffffffff813dca4c>] ext4_ext_map_blocks+0x13bc/0x1a50 ./fs/ext4/extents.c:4316
 [<ffffffff81395085>] ext4_map_blocks+0x415/0x7c0 ./fs/ext4/inode.c:628
 [<     inlined    >] ext4_writepages+0x8ef/0x1380 mpage_map_one_extent ./fs/ext4/inode.c:2152
 [<     inlined    >] ext4_writepages+0x8ef/0x1380 mpage_map_and_submit_extent ./fs/ext4/inode.c:2204
 [<ffffffff8139a13f>] ext4_writepages+0x8ef/0x1380 ./fs/ext4/inode.c:2531
 [<ffffffff81216beb>] do_writepages+0x4b/0x80 ./mm/page-writeback.c:2069
 [<ffffffff812c9e5a>] __writeback_single_inode+0x6a/0x380 ./fs/fs-writeback.c:444
 [<ffffffff812cb4bc>] writeback_sb_inodes+0x29c/0x6f0 ./fs/fs-writeback.c:668
 [<ffffffff812cb9cf>] __writeback_inodes_wb+0xbf/0x120 ./fs/fs-writeback.c:713
 [<ffffffff812cbe3b>] wb_writeback+0x40b/0x430 ./fs/fs-writeback.c:844
 [<     inlined    >] bdi_writeback_workfn+0x1d1/0x6a0 wb_do_writeback ./fs/fs-writeback.c:979
 [<ffffffff812ce6b1>] bdi_writeback_workfn+0x1d1/0x6a0 ./fs/fs-writeback.c:1024
 [<ffffffff8110b806>] process_one_work+0x2c6/0x740 ./kernel/workqueue.c:2196
 [<ffffffff8110cc2b>] worker_thread+0x25b/0x620 ./kernel/workqueue.c:2322
 [<ffffffff81116226>] kthread+0x126/0x130 kthread.c:0
 [<ffffffff8188eb6c>] ret_from_fork+0x7c/0xb0 ./arch/x86/kernel/entry_64.S:554

Allocated by thread T-1:


Memory state around the buggy address:
 ffff88000192bb00:  ........  ........  ........  ........
 ffff88000192bc00:  ........  ........  ........  ........
 ffff88000192bd00:  ........  ........  ........  ........
 ffff88000192be00:  ........  ........  ........  ........
 ffff88000192bf00:  ........  ........  ........  ........
>ffff88000192c000: >r<rrrrrrr rrrrrrrr  rrrrrrrr  ffffffff
 ffff88000192c100:  ffffffff  ffffrrrr  rrrrrrrr  rrrrrrrr
 ffff88000192c200:  rrrrrrrr  rrrrrrrr  rrrrrrrr  ffffffff
 ffff88000192c300:  ffffffff  ffffrrrr  rrrrrrrr  rrrrrrrr
 ffff88000192c400:  rrrrrrrr  rrrrrrrr  rrrrrrrr  ffffffff
 ffff88000192c500:  ffffffff  ffffrrrr  rrrrrrrr  rrrrrrrr
Legend:
 . - 8 allocated bytes
 f - freed bytes
 r - redzone bytes
 x=1..7 - x allocated bytes + (8-x) redzone bytes
```

```
=========================================================================
AddressSanitizer: heap-buffer-overflow on address ffff880000092640
Read of size 8 by thread T10488:
 [<     inlined    >] page_evictable+0x25/0x60 mapping_unevictable ././arch/x86/include/asm/bitops.h:329
 [<ffffffff8121fd75>] page_evictable+0x25/0x60 ./mm/vmscan.c:3681
 [<ffffffff812200a4>] shrink_page_list+0x244/0xf70 ./mm/vmscan.c:812
 [<ffffffff81221772>] shrink_inactive_list+0x232/0x7b0 ./mm/vmscan.c:1475
 [<     inlined    >] shrink_lruvec+0x539/0xa90 shrink_list ./mm/vmscan.c:1807
 [<ffffffff81222829>] shrink_lruvec+0x539/0xa90 ./mm/vmscan.c:2031
 [<ffffffff81222ede>] __shrink_zone+0x15e/0x360 ./mm/vmscan.c:2204
 [<ffffffff81223175>] shrink_zone+0x95/0xa0 ./mm/vmscan.c:2257
 [<     inlined    >] do_try_to_free_pages+0x1ca/0x910 shrink_zones ./mm/vmscan.c:2365
 [<ffffffff8122334a>] do_try_to_free_pages+0x1ca/0x910 ./mm/vmscan.c:2427
 [<ffffffff81223c2c>] try_to_free_pages+0x19c/0x290 ./mm/vmscan.c:2632
 [<     inlined    >] __alloc_pages_nodemask+0x9f8/0xdc0 __alloc_pages_direct_reclaim ./mm/page_alloc.c:2309
 [<     inlined    >] __alloc_pages_nodemask+0x9f8/0xdc0 __alloc_pages_slowpath ./mm/page_alloc.c:2583
 [<ffffffff81212a08>] __alloc_pages_nodemask+0x9f8/0xdc0 ./mm/page_alloc.c:2731
 [<     inlined    >] kmem_getpages+0x5f/0x1d0 __alloc_pages ./include/linux/gfp.h:307
 [<     inlined    >] kmem_getpages+0x5f/0x1d0 alloc_pages_exact_node ./include/linux/gfp.h:325
 [<ffffffff8126fecf>] kmem_getpages+0x5f/0x1d0 ./mm/slab.c:1758
 [<ffffffff81271d67>] fallback_alloc+0x177/0x250 ./mm/slab.c:3261
 [<ffffffff81271bbb>] ____cache_alloc_node+0x11b/0x150 ./mm/slab.c:3348
 [<     inlined    >] __kmalloc+0x36a/0x500 __do_cache_alloc ./mm/slab.c:3430
 [<     inlined    >] __kmalloc+0x36a/0x500 slab_alloc ./mm/slab.c:3462
 [<     inlined    >] __kmalloc+0x36a/0x500 __do_kmalloc ./mm/slab.c:3748
 [<ffffffff81272b2a>] __kmalloc+0x36a/0x500 ./mm/slab.c:3762
 [<     inlined    >] seq_read+0x254/0x7b0 kmalloc ./include/linux/slab.h:413
 [<ffffffff812c1624>] seq_read+0x254/0x7b0 ./fs/seq_file.c:235
 [<ffffffff8128c22a>] vfs_read+0xfa/0x240 ??:0
 [<ffffffff8128d4a2>] SyS_read+0x72/0xd0 ??:0
 [<ffffffff81843af5>] sysenter_dispatch+0x7/0x1a ./arch/x86/ia32/ia32entry.S:163

Allocated by thread T0:
 [<     inlined    >] setup_cpu_cache+0x196/0x2f0 kmalloc_node ./include/linux/slab.h:446
 [<ffffffff81820986>] setup_cpu_cache+0x196/0x2f0 ./mm/slab.c:2181
 [<ffffffff812746c5>] __kmem_cache_create+0x295/0x350 ./mm/slab.c:2399
 [<ffffffff81d0ea00>] create_boot_cache+0x73/0x9d ./mm/slab_common.c:309
 [<ffffffff81d0ea79>] create_kmalloc_cache+0x4f/0x84 ./mm/slab_common.c:326
 [<ffffffff81d0eafe>] create_kmalloc_caches+0x50/0x11a ./mm/slab_common.c:459
 [<ffffffff81d116da>] kmem_cache_init+0x34a/0x34f ./mm/slab.c:1629
 [<ffffffff81cdaed9>] start_kernel+0x1ef/0x4d7 ./init/main.c:468
 [<ffffffff81cda5e5>] x86_64_start_reservations+0x3a/0x3d ./arch/x86/kernel/head64.c:193
 [<ffffffff81cda75f>] x86_64_start_kernel+0x177/0x186 ./arch/x86/kernel/head64.c:182

The buggy address ffff880000092640 is located 0 bytes to the right
 of 128-byte region [ffff8800000925c0, ffff880000092640)

Memory state around the buggy address:
 ffff880000092100: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092200: rrrrrrrr ........ ........ rrrrrrrr
 ffff880000092300: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092400: ........ ........ rrrrrrrr rrrrrrrr
 ffff880000092500: rrrrrrrr rrrrrrrr rrrrrrrr ........
>ffff880000092600: ........ rrrrrrrr rrrrrrrr rrrrrrrr
                            ^
 ffff880000092700: rrrrrrrr rrrrrrrr ........ ........
 ffff880000092800: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092900: rrrrrrrr ........ ........ rrrrrrrr
 ffff880000092a00: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092b00: ........ ........ rrrrrrrr rrrrrrrr
Legend:
 f - 8 freed bytes
 r - 8 redzone bytes
 . - 8 allocated bytes
 x=1..7 - x allocated bytes + (8-x) redzone bytes
=========================================================================
=========================================================================
AddressSanitizer: heap-buffer-overflow on address ffff880000092648
Read of size 8 by thread T10488:
 [<ffffffff81220203>] shrink_page_list+0x3a3/0xf70 ./mm/vmscan.c:845
 [<ffffffff81221772>] shrink_inactive_list+0x232/0x7b0 ./mm/vmscan.c:1475
 [<     inlined    >] shrink_lruvec+0x539/0xa90 shrink_list ./mm/vmscan.c:1807
 [<ffffffff81222829>] shrink_lruvec+0x539/0xa90 ./mm/vmscan.c:2031
 [<ffffffff81222ede>] __shrink_zone+0x15e/0x360 ./mm/vmscan.c:2204
 [<ffffffff81223175>] shrink_zone+0x95/0xa0 ./mm/vmscan.c:2257
 [<     inlined    >] do_try_to_free_pages+0x1ca/0x910 shrink_zones ./mm/vmscan.c:2365
 [<ffffffff8122334a>] do_try_to_free_pages+0x1ca/0x910 ./mm/vmscan.c:2427
 [<ffffffff81223c2c>] try_to_free_pages+0x19c/0x290 ./mm/vmscan.c:2632
 [<     inlined    >] __alloc_pages_nodemask+0x9f8/0xdc0 __alloc_pages_direct_reclaim ./mm/page_alloc.c:2309
 [<     inlined    >] __alloc_pages_nodemask+0x9f8/0xdc0 __alloc_pages_slowpath ./mm/page_alloc.c:2583
 [<ffffffff81212a08>] __alloc_pages_nodemask+0x9f8/0xdc0 ./mm/page_alloc.c:2731
 [<     inlined    >] kmem_getpages+0x5f/0x1d0 __alloc_pages ./include/linux/gfp.h:307
 [<     inlined    >] kmem_getpages+0x5f/0x1d0 alloc_pages_exact_node ./include/linux/gfp.h:325
 [<ffffffff8126fecf>] kmem_getpages+0x5f/0x1d0 ./mm/slab.c:1758
 [<ffffffff81271d67>] fallback_alloc+0x177/0x250 ./mm/slab.c:3261
 [<ffffffff81271bbb>] ____cache_alloc_node+0x11b/0x150 ./mm/slab.c:3348
 [<     inlined    >] __kmalloc+0x36a/0x500 __do_cache_alloc ./mm/slab.c:3430
 [<     inlined    >] __kmalloc+0x36a/0x500 slab_alloc ./mm/slab.c:3462
 [<     inlined    >] __kmalloc+0x36a/0x500 __do_kmalloc ./mm/slab.c:3748
 [<ffffffff81272b2a>] __kmalloc+0x36a/0x500 ./mm/slab.c:3762
 [<     inlined    >] seq_read+0x254/0x7b0 kmalloc ./include/linux/slab.h:413
 [<ffffffff812c1624>] seq_read+0x254/0x7b0 ./fs/seq_file.c:235
 [<ffffffff8128c22a>] vfs_read+0xfa/0x240 ??:0
 [<ffffffff8128d4a2>] SyS_read+0x72/0xd0 ??:0
 [<ffffffff81843af5>] sysenter_dispatch+0x7/0x1a ./arch/x86/ia32/ia32entry.S:163

Allocated by thread T0:
 [<     inlined    >] setup_cpu_cache+0x196/0x2f0 kmalloc_node ./include/linux/slab.h:446
 [<ffffffff81820986>] setup_cpu_cache+0x196/0x2f0 ./mm/slab.c:2181
 [<ffffffff812746c5>] __kmem_cache_create+0x295/0x350 ./mm/slab.c:2399
 [<ffffffff81d0ea00>] create_boot_cache+0x73/0x9d ./mm/slab_common.c:309
 [<ffffffff81d0ea79>] create_kmalloc_cache+0x4f/0x84 ./mm/slab_common.c:326
 [<ffffffff81d0eafe>] create_kmalloc_caches+0x50/0x11a ./mm/slab_common.c:459
 [<ffffffff81d116da>] kmem_cache_init+0x34a/0x34f ./mm/slab.c:1629
 [<ffffffff81cdaed9>] start_kernel+0x1ef/0x4d7 ./init/main.c:468
 [<ffffffff81cda5e5>] x86_64_start_reservations+0x3a/0x3d ./arch/x86/kernel/head64.c:193
 [<ffffffff81cda75f>] x86_64_start_kernel+0x177/0x186 ./arch/x86/kernel/head64.c:182

The buggy address ffff880000092648 is located 8 bytes to the right
 of 128-byte region [ffff8800000925c0, ffff880000092640)

Memory state around the buggy address:
 ffff880000092100: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092200: rrrrrrrr ........ ........ rrrrrrrr
 ffff880000092300: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092400: ........ ........ rrrrrrrr rrrrrrrr
 ffff880000092500: rrrrrrrr rrrrrrrr rrrrrrrr ........
>ffff880000092600: ........ rrrrrrrr rrrrrrrr rrrrrrrr
                             ^
 ffff880000092700: rrrrrrrr rrrrrrrr ........ ........
 ffff880000092800: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092900: rrrrrrrr ........ ........ rrrrrrrr
 ffff880000092a00: rrrrrrrr rrrrrrrr rrrrrrrr rrrrrrrr
 ffff880000092b00: ........ ........ rrrrrrrr rrrrrrrr
Legend:
 f - 8 freed bytes
 r - 8 redzone bytes
 . - 8 allocated bytes
 x=1..7 - x allocated bytes + (8-x) redzone bytes
=========================================================================
```