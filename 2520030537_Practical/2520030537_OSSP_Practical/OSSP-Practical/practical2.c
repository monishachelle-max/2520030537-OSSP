#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>

int main()
{
int src,dest;
char buffer[1024];
int bytes;
src = open("input.txt", O_RDONLY);

if(src < 0)
{
printf("Cannot open source file\n");
return 1;
}
dest = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

while((bytes = read(src, buffer, sizeof(buffer))) > 0)
{
write(dest, buffer, bytes);
}
close(src);
close(dest);

printf("file copied successfully.\n");
return 0;
}
