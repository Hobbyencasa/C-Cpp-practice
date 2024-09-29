#include "..\Hashtable.cpp" //This comes with stdio and malloc

//Solution to "find two integers in a given array that
//add up to a given number"

void ArrayToHashTable(hashtable * Table, int * Arr, int length){
  for(int i = 0; i < length; ++i){
    listelement * element = AllocateListElement();
    InitializeListElement(element);
    (*element).key = (unsigned int)Arr[i];
    HashTableInsert(Table, element);
  }
}

void FindSum(hashtable * Table, int * Arr,
	     int arrlength, int sum, int result[2]){
  int current = 0;
  listelement * found;
  for(int i = 0; i < arrlength; ++i){
    current = Arr[i];
    if(sum-current > 0){
      found = HashTableSearch(Table, sum-current);
      if(found && (found != HashTableSearch(Table,current))){//Second condition is optional
	result[0] = current;
	result[1] = (*found).key;
      }
    }
  }
}

int main(){
  //int ExampleArray[10] = {12,5,6,1,8,8,9,3,44,27};
  int ExampleArray[10000] = {};
  for(int j = 0; j < 10000; ++j){
    ExampleArray[j] = j;
  }
  int length = sizeof(ExampleArray)/sizeof(ExampleArray[0]);

  hashtable ht = {NULL , 0};
  unsigned int slots = NUM_SLOTS;
  AllocateEntireHashTable(&ht, slots);
  ArrayToHashTable(&ht, ExampleArray, length);
  
  int Result[2] = {0, 0};
  
  char quit;
  puts("STARTNG... CONFIRM? Q TO QUIT");
  scanf(" %c",&quit);
  while( (quit != 'q') && (quit != 'Q') ){
    int sum = 0;
    puts("ENTER THE SUM TO BE FOUND");
    scanf(" %i",&sum);
    FindSum(&ht, ExampleArray, length, sum, Result);
    if(Result[0] != 0 && Result[1] != 0){
      printf("WE FOUND THIS PAIR: (%i, %i)\n",Result[0],Result[1]);
    }
    puts("AGAIN? Q TO QUIT");
    scanf(" %c",&quit);
  }
  FreeEntireHashTable(&ht);
  
  return 0;
}
