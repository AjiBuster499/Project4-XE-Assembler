#include <stdio.h>
#include <math.h>
int main(void) 
{
  char input[] = "1.0";
  int whole;
  int dec;
  int deci;
  int digits = 1;
  
  int count = 0;
  int divisor =1;
  long sum = 0;

  int bias = 1023;
  int exponent;
  

  long output = 0;

  sscanf(input, "%d.%d", &whole, &deci);
  printf("%d\n", whole);
  printf("%d\n", deci);
  dec = deci;

  if(whole == 0 && deci == 0)
  {
    printf("output = 0\n");
    return 0;
  }
  if (dec >= 10)
  {
    do
    {
      dec = dec - (dec * pow(10,digits));
      ++digits;
    }while(dec > 0);
  }
  printf("digits = %d\n",digits);
  dec = deci;
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
      if(dec * 2 < pow(10,digits))
      {
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > pow(10,digits))
      {
        dec = (dec *2) - pow(10,digits);
        sum = sum + pow(2,i-1);
        continue;
      }
      if(dec * 2 == pow(10,digits))
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
      if(dec * 2 < pow(10,digits))
      {
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > pow(10,digits))
      {
        dec = (dec *2) - pow(10,digits);
        sum = sum + pow(2,i-1);
        continue;
      }
      if(dec * 2 == pow(10,digits))
      {
        sum = sum + pow(2,i-1);
        break;
      }
    }
    output = output + sum;
  }
  else if(whole == 0)
  {
    for(int i = 36; i > 0; i--)//sets count
    {
      count = count + 1;
      if(dec * 2 < pow(10,digits))
      {
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > pow(10,digits))
      {
        break;
      }
      if(dec * 2 == pow(10,digits))
      {
        
        break;
      }
    }
    printf("count = %d\n",count);
    output = output + ((bias - count) * pow(2,36));//adds 11bit exponent into the output
    dec = deci;
    for(int i = 36; i > 0; i--)
    {
      if(dec * 2 < pow(10,digits))
      {
        
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > pow(10,digits))
      {
        dec = (dec *2) - pow(10,digits);
        sum = sum + pow(2,i-1);
        continue;
      }
      if(dec * 2 == pow(10,digits))
      {
        dec = 0;
        sum = sum + pow(2,i-1);
        output = output + 34359738368;
        printf("%ld", output);
        return output;
      }
    }
    printf("%ld\n",sum);
    sum = sum - pow(2,36 - count);
    sum = sum * pow(2,count);
  }
  for(int i = count ; i > 0; i--)
  {
    if(dec * 2 < pow(10,digits))
      {
        
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > pow(10,digits))
      {
        dec = (dec *2) - pow(10,digits);
        sum = sum + pow(2,i-1);
        continue;
      }
      if(dec * 2 == pow(10,digits))
      {
        dec = 0;
        sum = sum + pow(2,i-1);
        output = output + 34359738368;
        printf("%ld", output);
        return output;
      }
  }
  output = output + sum;
printf("%ld",output);
  return 0;
}
