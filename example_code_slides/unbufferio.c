#include <unistd.h>

int main()
{
   char buf[100];
   ssize_t n;
   // Copy standard input to standard output 
   while( (n=read(STDIN_FILENO, buf, 100)) != 0) { //讀取STDIN到buf中，回傳讀取char數量
      write(STDOUT_FILENO, buf, n);//把buf寫到STDOUT中
   }
   return 0;
}
