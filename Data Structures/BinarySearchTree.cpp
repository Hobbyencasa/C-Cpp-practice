//Implementation of a binary search tree.
#include <stdio.h>
#include <malloc.h>

#define MAX_KEYS 100

typedef struct bt_element{
  int key;
  struct bt_element * p; //Parent
  struct bt_element * left;
  struct bt_element * right;
}bt_element;

typedef struct bt{
  bt_element * root;
  int nkeys;
}bt;

void BSTInsert(bt * Tree, bt_element * element){
  bt_element * x = (*Tree).root;
  bt_element * y = NULL;
  while(x){
    y = x;
    if((*element).key < (*x).key){
      x = (*x).left;
    }else{
      x = (*x).right;
    }
  }
  (*element).p = y;
  if (!y){
    (*Tree).root = element;
  }
  else if ((*element).key < (*y).key){
    (*y).left = element;
  }
  else{
    (*y).right = element;
  }
}

void InOrderTreeWalk(bt_element * element){
  if(element){
    InOrderTreeWalk((*element).left);
    printf("KEY: %i, POINTER: %p\n",(*element).key, element);
    InOrderTreeWalk((*element).right);
  }
}

bt_element * AllocBSTElement(){
  bt_element * result = (bt_element*)malloc(sizeof(bt_element));
  (*result).key = 0;
  (*result).p = NULL;
  (*result).left = NULL;
  (*result).right = NULL;
  return result;
}

void AllocBSTArr(bt_element *** Arr, int n){
  *Arr = (bt_element **)malloc(n*sizeof(bt_element*));
}

void KeyArrToBSTElements(bt_element ** btarr, int * keys, int nkeys){
  for(int i = 0; i < nkeys; ++i){
    (*btarr[i]).key = keys[i];
  }
}

void BSTInsertAll(bt * tree, bt_element ** btarr){
  for(int i = 0; i < (*tree).nkeys; ++i){
    BSTInsert(tree, btarr[i]);
  }
}

bt_element * BSTSearchRecursive(bt_element * subroot, int key){
  if( !subroot || (*subroot).key == key){
    return subroot;
    }
  if(key < (*subroot).key){
    return BSTSearchRecursive((*subroot).left, key);
  }
  else{
    return BSTSearchRecursive((*subroot).right, key);
  }
}

bt_element * BSTSearchIterative(bt_element * subroot, int key){
  bt_element * result = subroot;
  while( result && (key != (*result).key)){
    if(key < (*result).key){
      result = (*result).left;
    }
    else{
      result = (*result).right;
    }
  }
  return result;
}

bt_element * BSTMin(bt_element * subroot){
  bt_element * result = subroot;
  while((*result).left){
    result = (*result).left;
  }
  return result;
}

bt_element * BSTMax(bt_element * subroot){
  bt_element * result = subroot;
  while((*result).right){
    result = (*result).right;
  }
  return result;
}

bt_element * BSTSuccessor(bt_element * subroot){
  if(subroot){
    if((*subroot).right){
      return BSTMin((*subroot).right);
    }
    else{
      bt_element * x = subroot;
      bt_element * y = (*subroot).p;
      while(y && (x == (*y).right)){
	x = y;
	y = (*y).p;
      }
      return y;
    }
  }
  return subroot;
}

bt_element * BSTPredecessor(bt_element * subroot){
  if(subroot){
    if((*subroot).left){
      return BSTMax((*subroot).left);
    }
    else{
      bt_element * x = subroot;
      bt_element * y = (*subroot).p;
      while(y && (x == (*y).left)){
	x = y;
	y = (*y).p;
      }
      return y;
    }
  }
  return subroot;
}

void Transplant(bt * Tree, bt_element * e1, bt_element * e2){
  if(!(*e1).p){
    (*Tree).root = e2;
  }
  else if (e1 == (*(*e1).p).left){
    (*(*e1).p).left = e2;
  }
  else{
    (*(*e1).p).right = e2;
  }
  if(e2){
    (*e2).p = (*e1).p;
  }
}

void BSTDeleteElement(bt * Tree, bt_element * element){
  if(!(*element).left){
    Transplant(Tree, element, (*element).right);
  }
  else if(!(*element).right){
    Transplant(Tree, element, (*element).left);
  }
  else{
    bt_element * y = BSTMin((*element).right);
    if(y != (*element).right){
      Transplant(Tree,y,(*y).right);
      (*y).right = (*element).right;
      (*(*y).right).p = y;
    }
    Transplant(Tree, element, y);
    (*y).left = (*element).left;
    (*(*y).left).p = y;
  }
  free(element);
}

/*
int main(){
  bt Tree;
  Tree.root = NULL;
  int keys[MAX_KEYS];
  for(int i = 0; i < MAX_KEYS; ++i){keys[i] = i;}//Example array of keys.
  int nKeys = sizeof(keys)/sizeof(keys[0]);
  Tree.nkeys = nKeys;
  bt_element ** ArrElements;
  AllocBSTArr(&ArrElements, nKeys);
  for(int i = 0; i < nKeys; ++i){
    ArrElements[i] = AllocBSTElement();
  }
  KeyArrToBSTElements(ArrElements, keys, nKeys);
  BSTInsertAll(&Tree, ArrElements);
  InOrderTreeWalk(Tree.root);
  bt_element * test = BSTSearchRecursive(Tree.root, 90);
  bt_element * test2 = BSTSearchIterative(Tree.root, 90);
  printf("RECURSIVE: %p\nITERATIVE: %p\n",test,test2);
  test = BSTMin(Tree.root);
  test2 = BSTMax(Tree.root);
  printf("MIN: %p\nMAX: %p\n",test,test2);
  test = BSTSearchIterative(Tree.root, 98);
  test2 = BSTSuccessor(test);
  printf("ORIGINAL: %p, SUCCESSOR: %p\n", test, test2);
  test2 = BSTPredecessor(test);
  printf("ORIGINAL: %p, PREDECESSOR: %p\n", test, test2);
  BSTDeleteElement(&Tree, test);
  InOrderTreeWalk(Tree.root);
  
  return 0;
}
*/
