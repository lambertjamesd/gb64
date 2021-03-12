
.data

# u32
#define osTask_type 0xFC0              
# u32
#define osTask_flags 0xFC4             
# u64*
#define osTask_ucode_boot 0xFC8        
# u32
#define osTask_ucode_boot_size 0xFCC   
# u64*
#define osTask_ucode 0xFD0             
# u32
#define osTask_ucode_size 0xFD4        
# u64*
#define osTask_ucode_data 0xFD8        
# u32
#define osTask_ucode_data_size 0xFDC   
# u64*
#define osTask_dram_stack 0xFE0        
# u32
#define osTask_dram_stack_size 0xFE4   
# u64*
#define osTask_output_buff 0xFE8       
# u64*
#define osTask_output_buff_size 0xFEC  
# u64*
#define osTask_data_ptr 0xFF0          
# u32
#define osTask_data_ptr_size 0xFF4     
# u64*
#define osTask_yield_data_ptr 0xFF8    
# u32
#define osTask_yield_data_size 0xFFC   