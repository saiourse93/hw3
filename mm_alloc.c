#include "mm_alloc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>

void *bottom = NULL;
struct log {
  size_t size;
  bool free;
  struct log* next;
  struct log* prev;
};
#define LOG_SIZE sizeof(struct log)
struct  log*  log2 (struct  log *prev, size_t size){
	
	struct  log *newLog;
	newLog = sbrk(0); 
	sbrk(size+LOG_SIZE); 
	if (prev){	
		prev->next=newLog;
		newLog->prev=prev;
	}
	newLog->size = size;
	newLog->next = NULL;
	newLog->free = false;

	return newLog;	
}
struct block* splitBlock(struct log* log1, size_t size){
	
	if (log1->size>=1+size+LOG_SIZE){  
		struct log * mlog = (struct log *)((char*)log1+LOG_SIZE+size); 
		mlog->size = log1->size-(size+LOG_SIZE); 
		mlog->free=true;
		mlog->next=log1->next;	
		mlog->prev=log1;
		if (log1->next){
			log1->next->prev=mlog;
		}
		log1->size=size;
		log1->next = mlog;
	}
	return log1;	
}

struct log* Free(struct log *prev, size_t size){	
	struct log *thislog = bottom;	

	while (true){	
		if (!thislog){	
			return log2(prev ,size);
		}
		if (thislog&&thislog->free && (thislog->size)>=size){	
			thislog=splitBlock(thislog,size);
			thislog->free=false;
			return thislog;	
		}
		prev = thislog; 
		thislog = thislog->next;	
	}
	return 0;	
}
void merge(struct log*thislog){	
	if (thislog->next){	
		if (thislog->next->free){	
			thislog->size+=thislog->next->size+LOG_SIZE;	
			if (thislog->next->next){	
				thislog->next->next->prev=thislog;
				thislog->next=thislog->next->next;
			}
		}
	}
	if (thislog->prev){
		if (thislog->prev->free){	
			merge(thislog->prev);
		}
	}
}


void* mm_malloc(size_t size)	
{
	struct log* newLog;
	if (bottom){	
		struct log* finallog=bottom;
		newLog=Free(finallog, size);
	}else{	
                newLog=log2(NULL,size);
		bottom=newLog;
	}
  return (void*) ((long) newLog+LOG_SIZE);

}



void* mm_realloc(void* ptr, size_t size)
{
void *newLogPtr = mm_malloc(size);	
	struct log* prevlog = (struct log*) ptr - 1;	
  memcpy(newLogPtr, ptr, prevlog->size);	 
  mm_free(ptr);	
  return newLogPtr;	


}

void mm_free(void* ptr)	
{
	struct log* thislog = (struct log*) ptr - 1;	
	thislog->free=true;	
	merge(thislog);	
}

