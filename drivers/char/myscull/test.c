#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

#define DEVICE "/dev/myscull"

int main () {
        int i,fd;
	char ch, write_buf[100],read_buf[100];
	
	fd=open(DEVICE,O_RDWR);

	if(fd==-1)
	printf("error opening the file\n");

	printf("file opened now what \n w-write \n r-read\n");
	scanf("%c",&ch);

	switch(ch) {
	case 'w':
		printf("enter data\n");
		scanf(" %[^\n]",write_buf);
		write(fd,write_buf,sizeof(write_buf));
		break;
	case 'r':
		read(fd,read_buf,sizeof(read_buf));
		printf("output %s \n",read_buf);
	}

	close(fd);

	return 0;


}
