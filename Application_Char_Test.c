#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE "/dev/Character_Device_Driver"

int main()
{
	int i, fd; //file descriptor
	char choice, writeToBuffer[100], readToBuffer[100];
	

	fd = open(DEVICE,O_RDWR);  //open for reading and writing
	
	if (fd==-1)
	{
		printf("file %s either does not exist or has been locked by another process\n",DEVICE);
		exit(-1);
	}
	printf("r = read from device\nw=writing to device\nenter command: ");
	scanf("%c",&choice);
	switch(choice)
	{
		case 'w':{
				printf("enter data: ");
				char tmp;
				scanf("%c",&tmp);
				scanf("%[^\n]",writeToBuffer);
				write(fd,writeToBuffer,sizeof(writeToBuffer));			
				break;
			}
		case 'r':{
				read(fd,readToBuffer,sizeof(readToBuffer));
				printf("device: %s\n",readToBuffer);
				break;
			}
		default:
				printf("command not recognized\n");
				break;
	}
	close(fd);

	return 0;
}
