/*
 * This code is written in C, but it could just as easily be done in C++.
 * The rand() and srand() functions are available in both languages.
 */
 
#include <stdio.h>  
#include <stdlib.h>  
#include <time.h>  

int randomgenerator()
{
  int i;
  genagain:
  srand(time(NULL));
  
  i = rand();

  if((i/100000>9999)&&(i/100000<99999)){
  
  return i/100000;
  }
  else
	{
	  goto genagain;
	}



}