#include <stdio.h>
#include <math.h>
int main(void) 
{
  char input[] = "-1.6";
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
  dec = deci;

  if(whole == 0 && deci == 0)
  {
    return 0;//this will return zero when the input is 0.0
  }
  //this calculates the number of digits of the whole number if it is more than 1
  if (dec >= 10)
  {
    do
    {
      dec = dec - (dec * pow(10,digits));
      ++digits;
    }while(dec > 0);
  }
  
  dec = deci;
  //this sets the first bit to 1 if the number is negative
  if(whole < 0)
  {
    output = output + 140737488355328;
    whole = whole * -1;
  }
  //calculates output for numbers with an absolute value that is >= 2
  if(whole > 1)
  {
  
    while(whole >= divisor)
    {
      count = count + 1;
      divisor = divisor * 2;

    }
    count = count - 1;
    exponent = bias + (count);
    output = output + (exponent * pow(2,36));
    output = output + ((whole - pow(2,count)) * pow(2, 36-(count)));//adds the remainder of the whole number to the 36bit section
    
    //calculates the value of the decimal
    for(int i = 35 - count; i >= 0; i--)
    {
      if(dec * 2 < pow(10,digits))
      {
        dec = dec * 2;
        continue;
      }
      if(dec * 2 > pow(10,digits))
      {
        dec = (dec *2) - pow(10,digits);
        sum = sum + pow(2,i);
        continue;
      }
      if(dec * 2 == pow(10,digits))
      {
        sum = sum + pow(2,i);
        break;
      }

    }
    output = output + sum;
  }
  //calculates output for numbers with an absolute value between 1 and 2
  else if(whole == 1)
  {
    output = output + (bias * pow(2,36));//sets the 11bit exponent to 1023
    
    //calculates the value of the decimal
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

  //calculates output for numbers that have an absolute value between 0 and 1
  else if(whole == 0)
  {
    for(int i = 36; i > 0; i--)//sets count which is the exponent
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
    output = output + ((bias - count) * pow(2,36));//adds 11bit exponent into the output
    dec = deci;

    //calculates the decimal number
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
        return output;
      }
    }
    sum = sum - pow(2,36 - count);//sets the first bit that is a one to a zero
    sum = sum * pow(2,count);//shifts the number to the left

    //continues filling in the remaining lower numbers that were not filled in due to the shift
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
        return output;
      }
    }
    output = output + sum;
  }
  
  return output;
}
