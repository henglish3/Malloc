//Harrison English
//902113676
#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the 
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you will receive a 20 point deduction. You have been
 * warned.
 */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)
#endif


/* make sure this always points to the beginning of your current
 * heap space! if it does not, then the grader will not be able
 * to run correctly and you will receive 0 credit. remember that
 * only the _first_ call to my_malloc() returns the beginning of
 * the heap. sequential calls will return a pointer to the newly
 * added space!
 * Technically this should be declared static because we do not
 * want any program outside of this file to be able to access it
 * however, DO NOT CHANGE the way this variable is declared or
 * it will break the autograder.
 */
void* heap;

/* our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * Technically this should be declared static for the same reasons
 * as above, but DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t* freelist[8];
/**** SIZES FOR THE FREE LIST ****
 * freelist[0] -> 16
 * freelist[1] -> 32
 * freelist[2] -> 64
 * freelist[3] -> 128
 * freelist[4] -> 256
 * freelist[5] -> 512
 * freelist[6] -> 1024
 * freelist[7] -> 2048
 */


void* my_malloc(size_t size)
{

	//Ups the size requested to the next size cap
	int size_needed = find_size_needed(size);
	//test to see if the size is greater than 2048 or less than 0
	//and returns NULL if so
	if(size_needed == -1)
		return NULL;

	//if the heap is empty it calls sbrk and sets up the freelist
	if(heap == NULL) 
	{
		heap = my_sbrk(SBRK_SIZE);
		freelist[7] = (metadata_t *) heap;
		freelist[7]->size = 2048;
		freelist[7]->next = NULL;
		freelist[7]->prev = NULL;
		freelist[7]->in_use = 0;
	}


	//finds the index of the freelist based off the size of the malloc
	int i = find_index(size_needed);
	
	//grabs the freelist at the index and puts it into a variable for use
	metadata_t* memBlock = freelist[i];	

	//test to see if the freelist has available memory in that spot
	if(freelist[i] != NULL)
	{
		//sets the free list to it's next member which may be null
		freelist[i] = freelist[i]->next;
		//pops the memory block we will use off the free list and sets
		//its in_use to true (1)
		memBlock->next = NULL;	
		memBlock->in_use = 1;

		//returns a pointer to the memory block after the metadata_t
		return ((char *)memBlock) + sizeof(metadata_t);
	}
	else
	{
		//grabs the index after the index we need
		int j = i + 1;
		//loops through the freelist looking for the first available memory
		//greater than the index we need
		while (j < 8 && freelist[j] == NULL)
		{
			j ++;
		}
		
		//test if there is no available memory
		if(j == 8)
		{
			//grabs more memory for the heap using sbrk
			freelist[7] = my_sbrk(SBRK_SIZE);
			//adds it to the free list
			freelist[7]->size = 2048;
			freelist[7]->next = NULL;
			freelist[7]->prev = NULL;
			freelist[7]->in_use = 0;
			//returns the index available to the last spot
			j = 7;
		}
	
		//sets our memory Block to the available block
		memBlock = freelist[j];
		//creates a place for the new memory block to be broken in halves
		metadata_t* newMemBlock;

		//loops until the memory block we have matches the one we need
		while (i != j)
		{
			//sets the free list to equal the next in line
			freelist[j] = freelist[j]->next;

			//cuts the memory block in half
			memBlock->size = memBlock->size/2;
			//creates it's buddy's metadata half way through the block
			newMemBlock = (metadata_t *)((char *) memBlock + memBlock->size);
			//sets the memoryblock to point at it's buddy and equate the size
			memBlock->next = newMemBlock;
			newMemBlock->prev = memBlock;
			newMemBlock->size = memBlock->size;
		
			//decrements the index
			j--;
		
			//sets the next index's freelist to the new memory blocks
			freelist[j] = memBlock; 

		}
	
		//grabs the memory block
		memBlock = freelist[i];
	
		//pops the memory block off the freelist
		freelist[i] = freelist[i]->next;
		//sets all of it's data to show in use
		memBlock->next = NULL;	
		memBlock->in_use = 1;

		//return the memory block grabed to the user
		return ((char *) memBlock) + sizeof(metadata_t);
	}
}
//places the size into the blocks in the free list
int find_size_needed(int size)
{
	//adds the size requested with the size of the metadata
	int size_needed = size+sizeof(metadata_t);
  	
	//test for where it belongs in the freelist
	if(size_needed > 2048 || size_needed < 0)
  		return -1;
	else if (size_needed > 1024)
		size_needed = 2048;
	else if (size_needed > 512)
		size_needed = 1024;
	else if (size_needed > 256)
		size_needed = 512;
	else if (size_needed > 128)
		size_needed = 256;
	else if (size_needed > 64)
		size_needed = 128;
	else if (size_needed > 32)
		size_needed = 64;
	else if (size_needed > 16)
		size_needed = 32;
	else
		size_needed = 16;

	//gives the new blocked size back to caller
	return size_needed;
}

//translates the size into an index for the freelist
int find_index(int size_indexed)
{
	//index
	int i = 0;
	//starting size amount
	int amount = 16;
	
	while (size_indexed != amount)
	{
		//increments the index
		i++;
		//increments the size
		amount *= 2;
	}

	//returns the index to the caller
	return i;
}

void* my_realloc(void* ptr, size_t new_size)
{
	//test for special case where ptr is null which is just a malloc
	if(ptr == NULL)
  		return my_malloc(new_size);
	//test for special case where size is 0 which is just a free
	if(new_size == 0)
	{
		my_free(ptr);
		return NULL;
	}
	
	//all other calls it makes a new malloc
	void* new_ptr = my_malloc(new_size);
	//grabs the orig ptrs metadata
	metadata_t* meta_ptr = (metadata_t *)((char *)ptr - sizeof(metadata_t));
	//copies any info in the old memory block into the new one
	my_memcpy(new_ptr, ptr, meta_ptr->size);
	//frees the old block
	my_free(ptr);
	
	//returns the new block
	return new_ptr;
}

void my_free(void* ptr)
{
	//grabs the metadata of the ptr
	metadata_t* memBlock = (metadata_t *)((char *) ptr-sizeof(metadata_t));
	//sets it's in use back to false (0)
	memBlock->in_use = 0;

	//find the buddy of the ptr
	metadata_t* buddy = find_buddy(memBlock);
	
	//loop through as long as the buddy matches the memory block and is available
	while(buddy != NULL && buddy->size == memBlock->size && buddy->in_use == 0 && memBlock->size != 2048)
	{
		
		int index;
		//test to see which comes first buddy or the memory block
		if(memBlock < buddy)
		{
			//doubles the size to make up for merge
			memBlock->size = memBlock->size*2;
			//finds the index of the buddy
			index = find_index(buddy->size);
			//pops the buddy off of the freelist
			freelist[index] = freelist[index]->next;
			//moves pointer off of metadata for garabage collection
			buddy = NULL;
		}
		else
		{	
			//doubles of size for merge
			buddy->size = buddy->size*2;
			//finds size of memory block cause it is bigger
			index = find_index(memBlock->size);
			//pops buddy off of the frelist
			freelist[index] = freelist[index]->next;
			//sets the buddy to memory block becuase it has the smaller address
			memBlock = buddy;
			//moves pointer off of metadata for garabage collection
			buddy = NULL;
		}
		
		//finds the next body of the memory block
		buddy = find_buddy(memBlock);
	}

	//finds the index of the memory block
	int i = find_index(memBlock->size);
	//sets the memory blocks next to whatever is on the freelist
	memBlock->next = freelist[i];
	//sets the appropriate freelist space to the newly freed memory block
	freelist[i] = memBlock;
}

metadata_t* find_buddy(metadata_t* ptr)
{
	//XORs the address of the pointer with it's size to find it's buddy
  	return (metadata_t *)((int)ptr^(ptr->size));
}

void* my_memcpy(void* dest, const void* src, size_t num_bytes)
{
	//creates char points for the void pointers so it grabs the right size
	//of data when copying
	char * destPtr = (char *) dest;
	char * srcPtr = (char *) src;
	//loops through the amount of data to be copies
	for(int i = 0; i < num_bytes; i++) 
 	{
		//copies from the src to dest
  		destPtr[i] = srcPtr[i];
	}

	//returns the new pointer
  return destPtr;
}
