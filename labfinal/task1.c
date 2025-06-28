#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <stdbool.h>
int main ()
{ 
int arr_len1,arr_len2;
  char str[100];
  int elem ;
  bool subset = true;
  printf("Please enter the length of array1:");
  scanf("%d",&arr_len1);
  int arr1[arr_len1];
  
  printf("Please enter the elements of arr1:");
  for (int i=0;i<arr_len1;i++){
 	 scanf("%d",&elem);
  	 arr1[i]=elem;
  }
  printf(" Please enter the length of array2:");
  scanf("%d",&arr_len2);
  int arr2[arr_len2];
  bool contains[arr_len2];
  
  printf("Please enter the elements of array2:");
  for (int i=0;i<arr_len2;i++){
 	 scanf("%d",&elem);
 	 for (int a=0;a<arr_len1;a++){
  	   if (arr1[i]==elem);{
  	        contains[i]=true;
  	      }
  } 
          arr2[i]=elem;
  }
  
  for (int i=0;i<arr_len2;i++){
	if (contains[i]==false){
		subset=false;
  	      }
 }
 if (subset){
 	printf("Array2 is a subset of Arr1\n");
 }
 else {
 	printf("Array2 not subset of Arr1\n");
}
return 0;
}
