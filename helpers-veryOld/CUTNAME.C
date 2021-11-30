#include <stdio.h>
#include <string.h>

int main(int argc,char* argv[])
{
char name[255];
char ext[255];
char *pos=NULL;
int name_len,ext_len,len;

if(argc<=2)
	{
	fprintf(stderr,"use: %s N[.X] filename\n\n",argv[0]);
	fprintf(stderr," where N - number of char required in name\n"
		       " and   X - number of char required in extension.\n");
	exit(1);
	}
len=strlen(argv[2]);
strcpy(name,argv[1]);
if((pos=strchr(name,'.'))!=NULL)
	{
	*pos='\0';
	strcpy(ext,pos+1);
	}
/*printf("%s - %s\n",name,ext);*/
name_len=atoi(name);
ext_len=atoi(ext);

if((pos=strchr(argv[2],'.'))!=NULL)
	{
  print("%d\n",pos-argv[2]);
  if(pos-name<name_len)
	{
	name_len=pos-argv[2];
	printf("NN %d",name_len);
	}
	}
if(name_len>len) 
	name_len==len;

if(name_len+ext_len>len)
	ext_len=0;

strncpy(name,argv[2],name_len);
name[name_len]='\0';
if((pos=strrchr(argv[2],'.'))!=NULL)
	strncpy(ext,pos,ext_len);
ext[ext_len]='\0';
/*printf("%d - %d\n",name_len,ext_len);*/
printf("%s%s",name,ext);
} 
