//Implementation of a hash table that resolves collisions by chaining.
//Keys are nonnegative integers.

//TO DO: free memory (lists and hashtables), make insert and delete operations.
//UPDATE: looks like the insertion operation works.
//UPDATE: freeing memory complete (I think) and delete operation seems to work.

#include <stdio.h>
#include <malloc.h>

#define HASH_CONSTANT 8
#define NUM_SLOTS 256 //2**6 = 2**HASH_CONSTANT
#define MACHINE_WORD_BITS 16 //Arbitrary choice, I bet it could be bigger

typedef struct listelement{
  unsigned int key;
  int value;
  struct listelement * next;
  struct listelement * prev;
}listelement;

typedef struct hashtable{
  listelement ** list;
  unsigned int slots;
}hashtable;

void InitializeListElement(listelement * element);//Move this to a header file

int HashFunction(unsigned int key){
  int a = 33; //Arbitrary choice
  int pow = 65536; //2**16 = 2**MACHINE_WORD_BITS
  return ( (key*a)%pow )>>(MACHINE_WORD_BITS - HASH_CONSTANT);
}

void AllocateHashTableSlots(hashtable * Table, unsigned int slots){
  (*Table).slots = slots;
  (*Table).list = (listelement**)malloc(slots * sizeof(listelement*));
  for(int i = 0; i < slots; ++i){
    (*((*Table).list+i)) = NULL;
  }
}

void AllocateHashTableElement(hashtable * Table, int index){
  if((index >= 0) && (index < (*Table).slots)){
    (*((*Table).list+index)) = (listelement *)malloc(sizeof(listelement));
    InitializeListElement((*((*Table).list+index)));
  }
  else{
    puts("BRUH.");
  }
}

listelement * AllocateListElement(){
  return (listelement*)malloc(sizeof(listelement));
}

void AllocateEntireHashTable(hashtable * Table,int slots){
  AllocateHashTableSlots(Table, slots);
  for(int i = 0; i < (*Table).slots; ++i){
    AllocateHashTableElement(Table, i);
  }
}

void HashTablePrepend(hashtable * Table, listelement * element,int index){
  (*element).next = (*Table).list[index];
  (*(*Table).list[index]).prev = element;
  (*Table).list[index] = element;
}

void InitializeListElement(listelement * element){
  (*element).key = 0;
  (*element).value = 0;
  (*element).next = NULL;
  (*element).prev = NULL;
}

listelement * ListSearch(listelement * from, unsigned int key){
  do{
    if ( (*from).key == key){
      return from;
    }
    from = (*from).next;
  }while(from);
  return NULL;
}

listelement * HashTableSearch(hashtable * Table, unsigned int key){
  int index = HashFunction(key);
  listelement * result = ListSearch((*Table).list[index],key);
  return result;
}

void HashTableInsert(hashtable * Table, listelement * element){
  HashTablePrepend(Table, element, HashFunction((*element).key));
}

void FreeListElement(listelement * element){
  free((void*)element);
}

void FreeEntireHashTable(hashtable * Table){
  for(int i = 0; i < (*Table).slots; ++i){
    listelement * current = (*Table).list[i];
    listelement * next = NULL;
    if((*current).next){
      next = (*current).next;
    }
    //printf("CUURENT: %p\nNEXT: %p\nSLOT: %i\n",current,next,i);
    while(current){
      //puts("ENTERED WHILE");
      FreeListElement(current);
      current = next;
      if(next){
	next = (*next).next;
      }
      //printf("CURRENT: %p\n NEXT: %p\n",current,next);
    }
  }
  //puts("FINISHED FOR LOOP");
  free((void*)(*Table).list);
  (*Table).slots = 0;
  //puts("FREED TABLE");
}

void HashTableDeleteElement(hashtable * Table, unsigned int key){
  listelement * ToBeDeleted = HashTableSearch(Table, key);
  if((*ToBeDeleted).next){
    // puts("ENTERED FIRST IF");
    (*(*ToBeDeleted).next).prev = (*ToBeDeleted).prev;
    //puts("EXITED FIRST IF");
  }
  //puts("PASSED FIRST IF");
  if((*ToBeDeleted).prev){
    //puts("ENTERED SECOND IF");
    (*(*ToBeDeleted).prev).next = (*ToBeDeleted).next;
    //puts("EXITED SECOND IF");
  }
  //puts("PASSED SECOND IF");
  else{
    //puts("ENTERED ELSE");
    (*Table).list[HashFunction(key)] = (*ToBeDeleted).next;
    //puts("EXITED ELSE");
  }
  //puts("PASSED ELSE");
  FreeListElement(ToBeDeleted);
  //puts("PASSED FREE ELEMENT");
}

void PrintHashTable(hashtable * Table){
  for(int i = 0; i < (*Table).slots; ++i){
    puts("WE'RE IN");
    unsigned int key = (*(*Table).list[i]).key;
    int value = (*(*Table).list[i]).value;
    listelement * next = (*(*Table).list[i]).next;
    listelement * prev = (*(*Table).list[i]).prev;
    printf("KEY: %u\nVALUE: %i\nNEXT: %p\nPREV: %p\nITERATION: %i\n",key,value,next,prev,i);
    while(next){
      key = (*next).key;
      value = (*next).value;
      prev = (*next).prev;
      next = (*next).next;
      printf("KEY: %u\nVALUE: %i\nNEXT: %p\nPREV: %p\nITERATION: %i\n",key,value,next,prev,i);
    } 
  }
}


int main(){

  hashtable Table = {NULL, 0};
  unsigned int slots = NUM_SLOTS;
  AllocateEntireHashTable(&Table, slots);
  
  //PrintHashTable(&Table);
  listelement * Element = AllocateListElement();
  InitializeListElement(Element);
  (*Element).key = 1901;
  HashTableInsert(&Table,Element);
  
  listelement * Element2 = AllocateListElement();
  InitializeListElement(Element2);
  (*Element2).key = 1902;
  HashTableInsert(&Table,Element2);
  
  listelement * bruh = HashTableSearch(&Table,2);
  printf("REAL: %p\nOBTAINED: %p\n",Element, bruh);

  listelement * Element3 = AllocateListElement();
  InitializeListElement(Element3);
  (*Element3).key = 1900;
  HashTableInsert(&Table,Element3);
  printf("Hash of key %u is %i\n",(*Element3).key, HashFunction((*Element3).key));
  
  PrintHashTable(&Table);
  puts("SPACE TO SEE \n\n\n");

  HashTableDeleteElement(&Table, 1901);
  PrintHashTable(&Table);
  puts("MORE SPACE \n\n\n");
  
  FreeEntireHashTable(&Table);
  puts("FINISHED FREEING TABLE");
  
  return 0;
}
