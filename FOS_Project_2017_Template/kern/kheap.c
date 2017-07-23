#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

//=================================================================================//
//============================ REQUIRED FUNCTION ==================================//
//=================================================================================//
struct meta_data
{
	uint32 address;
	uint32 size;
	int exist;
}arr[PAGE_SIZE];
int array_size = -1;

uint32 next_fit_address = KERNEL_HEAP_START;
uint32 curr_heap_start = KERNEL_HEAP_START;
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//panic("kmalloc() is not implemented yet...!!");
	//NOTE: Allocation is continuous increasing virtual address
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	//TODO: [PROJECT 2017 - BONUS1] Implement a Kernel allocation strategy
	// Instead of the continuous allocation/deallocation, implement both
	// FIRST FIT and NEXT FIT strategies
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	if(isKHeapPlacementStrategyFIRSTFIT())
	{
			//TODO: [PROJECT 2017 - BONUS1] Implement a Kernel allocation strategy
			// Instead of the continuous allocation/deallocation, implement both
			// FIRST FIT and NEXT FIT strategies
			for(uint32 start = KERNEL_HEAP_START; start < KERNEL_HEAP_MAX; start += PAGE_SIZE)
			{
				if(size > KERNEL_HEAP_MAX-start)
				{
					return NULL;
				}
				uint32 *p = NULL;
				struct Frame_Info *f = NULL;
				uint32 tmp = start;
				int check = 0;
				for(int n = 0; n < size; n += PAGE_SIZE)
				{
					f = get_frame_info(ptr_page_directory, (void*)tmp, &p);
					if(f != NULL)
					{
						check = 1;
						break;
					}
					tmp += PAGE_SIZE;
				}
				if(check == 0)
				{
					uint32 curr_heap_start_1 = start;
					for(uint32 x = 0; x < size; x+=PAGE_SIZE)
					{
						struct Frame_Info *fi ;
						int ret = allocate_frame(&fi);
						if (ret == E_NO_MEM)
						{
							for(uint32 j = 0; j < x; j+=PAGE_SIZE)
							{
								curr_heap_start_1 -= PAGE_SIZE;
								unmap_frame(ptr_page_directory, (void*)curr_heap_start_1);
							}
							return NULL;
						}
						ret = map_frame(ptr_page_directory, fi, (void*)curr_heap_start_1, PERM_WRITEABLE |PERM_PRESENT);
						if (ret == E_NO_MEM)
						{
							free_frame(fi) ;
							for(uint32 j = 0; j < x; j+=PAGE_SIZE)
							{
								curr_heap_start_1 -= PAGE_SIZE;
								unmap_frame(ptr_page_directory, (void*)curr_heap_start_1);
							}
							return NULL;
						}
						curr_heap_start_1 += PAGE_SIZE;
					}
					array_size++;
					arr[array_size].address = start;
					arr[array_size].size = size;
					arr[array_size].exist = 1;
					return (void*)start;
				}
			}
	}
	else if(isKHeapPlacementStrategyNEXTFIT())
	{
		// Implement next fit
		int count = 0;
		int totalSize = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE;
		for(uint32 start = next_fit_address; start < KERNEL_HEAP_MAX; start += PAGE_SIZE)
		{
			if(count >= KERNEL_HEAP_MAX)
			{
				break;
			}
			if(size > KERNEL_HEAP_MAX-start)
			{
				next_fit_address = KERNEL_HEAP_START;
				start = KERNEL_HEAP_START;
			}
			uint32 *p = NULL;
			struct Frame_Info *f = NULL;
			uint32 tmp = start;
			int check = 0;
			for(int n = 0; n < size; n += PAGE_SIZE)
			{
				f = get_frame_info(ptr_page_directory, (void*)tmp, &p);
				if(f != NULL)
				{
					check = 1;
					break;
				}
				tmp += PAGE_SIZE;
			}
			if(check == 0)
			{
				uint32 curr_heap_start_1 = start;
				for(uint32 x = 0; x < size; x+=PAGE_SIZE)
				{
					struct Frame_Info *fi ;
					int ret = allocate_frame(&fi);
					if (ret == E_NO_MEM)
					{
						for(uint32 j = 0; j < x; j+=PAGE_SIZE)
						{
							curr_heap_start_1 -= PAGE_SIZE;
							unmap_frame(ptr_page_directory, (void*)curr_heap_start_1);
						}
						return NULL;
					}
					ret = map_frame(ptr_page_directory, fi, (void*)curr_heap_start_1, PERM_WRITEABLE |PERM_PRESENT);
					if (ret == E_NO_MEM)
					{
						free_frame(fi) ;
						for(uint32 j = 0; j < x; j+=PAGE_SIZE)
						{
							curr_heap_start_1 -= PAGE_SIZE;
							unmap_frame(ptr_page_directory, (void*)curr_heap_start_1);
						}
						return NULL;
					}
					curr_heap_start_1 += PAGE_SIZE;
					count+=PAGE_SIZE;
				}
				array_size++;
				arr[array_size].address = start;
				arr[array_size].size = size;
				arr[array_size].exist = 1;
				next_fit_address = start + PAGE_SIZE;
				return (void*)start;
			}
			count+=PAGE_SIZE;
		}
	}
	else
	{
		if(size > KERNEL_HEAP_MAX-curr_heap_start)
			{
				return NULL;
			}
			int check = 0;
			for(int i = 0; i < size; i+=PAGE_SIZE)
			{
				uint32 *ptr = NULL;
				struct Frame_Info* f1 = get_frame_info(ptr_page_directory ,(void*)(curr_heap_start+i),&ptr );
				if(f1 != NULL)
				{
					check = 1;
					break;
				}
			}
			if(check == 0)
			{
				uint32 start = curr_heap_start;
				for(uint32 x = 0; x < size; x+=PAGE_SIZE)
				{
					struct Frame_Info *fi ;
					int ret = allocate_frame(&fi);
					if (ret == E_NO_MEM)
					{
						for(uint32 j = 0; j < x; j+=PAGE_SIZE)
						{
							curr_heap_start -= PAGE_SIZE;
							unmap_frame(ptr_page_directory, (void*)curr_heap_start);
						}
						return NULL;
					}
					ret = map_frame(ptr_page_directory, fi, (void*)curr_heap_start, PERM_WRITEABLE |PERM_PRESENT);
					if (ret == E_NO_MEM)
					{
						free_frame(fi) ;
						for(uint32 j = 0; j < x; j+=PAGE_SIZE)
						{
							curr_heap_start -= PAGE_SIZE;
							unmap_frame(ptr_page_directory, (void*)curr_heap_start);
						}
						return NULL;
					}
					curr_heap_start += PAGE_SIZE;
				}
				array_size++;
				arr[array_size].address = start;
				arr[array_size].size = size;
				arr[array_size].exist = 1;
				return (void*)start;
			}
	}
	return NULL ;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

	int index = 0;
	while(index <= array_size)
	{
		if(arr[index].address == (int)virtual_address && arr[index].exist == 1)
		{
			uint32 curr_address = (int)virtual_address;
			for(int k = 0; k < arr[index].size; k+=PAGE_SIZE)
			{
				unmap_frame(ptr_page_directory, (void*)curr_address);
				curr_address+=PAGE_SIZE;
			}
			arr[index].exist = 0;
			break;
		}
		index++;
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

		unsigned int  i;
		unsigned int tmp;
		for(i = KERNEL_HEAP_START; i<KERNEL_HEAP_MAX ; i+=PAGE_SIZE)
		{

			tmp =  kheap_physical_address(i);
			if (tmp == physical_address)
				return i;

		}
			return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	uint32 *ptr_page_table = NULL;
	get_page_table(ptr_page_directory, (void*)virtual_address, &ptr_page_table);
	if (ptr_page_table  !=  NULL)
	{
		int avalible = ptr_page_table[PTX(virtual_address)] & PERM_PRESENT;
		if(avalible == 1)
		{
			unsigned int tmp1 = ptr_page_table[PTX(virtual_address)] >> 12;
			tmp1 = tmp1 * PAGE_SIZE;
			unsigned int tmp2 = virtual_address << 20;
			tmp2 = tmp2 >> 20;
			return tmp1 + tmp2;
		}
	}
	//change this "return" according to your answer
	return 0;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2017 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");

}
