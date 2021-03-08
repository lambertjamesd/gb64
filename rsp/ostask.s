

.symbol osTask_type, 0xFC0              # u32
.symbol osTask_flags, 0xFC4             # u32
.symbol osTask_ucode_boot, 0xFC8        # u64*
.symbol osTask_ucode_boot_size, 0xFCC   # u32
.symbol osTask_ucode, 0xFD0             # u64*
.symbol osTask_ucode_size, 0xFD4        # u32
.symbol osTask_ucode_data, 0xFD8        # u64*
.symbol osTask_ucode_data_size, 0xFDC   # u32
.symbol osTask_dram_stack, 0xFE0        # u64*
.symbol osTask_dram_stack_size, 0xFE4   # u32
.symbol osTask_output_buff, 0xFE8       # u64*
.symbol osTask_output_buff_size, 0xFEC  # u64*
.symbol osTask_data_ptr, 0xFF0          # u64*
.symbol osTask_data_ptr_size, 0xFF4     # u32
.symbol osTask_yield_data_ptr, 0xFF8    # u64*
.symbol osTask_yield_data_size, 0xFFC   # u32