#include <stdio.h>
#include <math.h>
int main(void) 
{
  char input[] = "0.6";
  int whole;
  int dec;
  
  int count = 0;
  int divisor =1;
  long sum = 0;

  int bias = 1023;
  
  int exponent;
  int fraction;

  long output = 0;

  sscanf(input, "%d.%d", &whole, &dec);
  printf("%d\n", whole);
  printf("%d\n", dec);
  if(whole < 0)
  {
    output = output + 140737488355328;
    whole = whole * -1;
  }
  if(whole >= 1)
  {
  
    while(whole >= divisor)
    {
      count = count + 1;
      divisor = divisor * 2;

    }
    printf("%d\n",count);
    exponent = bias + (count - 1);
    output = output + (exponent * pow(2,36));
    output = output + ((whole - pow(2,count-1)) * pow(2, 36-(count-1)));
    for(int i = 36-(count-1); i > 0; i--)
    {
      if(dec * 2 < 10)
      {
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > 10)
      {
        dec = (dec *2) - 10;
        sum = sum + pow(2,i-1);
        continue;
      }
      if(dec * 2 == 10)
      {
        sum = sum + pow(2,i-1);
        break;
      }

    }
    printf("%ld\n",sum);
    output = output + sum;
  }
  else if(whole == 1)
  {
    output = output + (bias * pow(2,36));
    for(int i = 36; i > 0; i--)
    {
      if(dec * 2 < 10)
      {
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > 10)
      {
        dec = (dec *2) - 10;
        sum = sum + pow(2,i-1);
        continue;
      }
      if(dec * 2 == 10)
      {
        sum = sum + pow(2,i-1);
        break;
      }
    }
    output = output + sum;
  }
  else if(whole == 0)
  {
    for(int i = 36; i > 0; i--)
    {
      count = count + 1;
      if(dec * 2 < 10)
      {
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > 10)
      {
        break;
      }
      if(dec * 2 == 10)
      {
        
        break;
      }
    }
    printf("count = %d\n",count);
    output = output + ((bias - count) * pow(2,36));
    int tempcount = count;
    for(int i = 36 + (count - 1); i > 0; i--)
    {
      if(dec * 2 < 10)
      {
        
        dec = dec * 2;
        tempcount = tempcount -1;
        continue;
      }
      if(dec * 2 > 10)
      {
        dec = (dec *2) - 10;
        if(tempcount <= 0)
        {
          sum = sum + pow(2,i-1);
        }
        tempcount = tempcount - 1;
        continue;
      }
      if(dec * 2 == 10)
      {
        output = output + 34359738368;
        break;
      }
    }
    output = output + sum;
  }
printf("%ld",output);
  return 0;
}