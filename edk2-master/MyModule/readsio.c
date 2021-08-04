// readsio.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include"Library\IoLib.h"
#include<stdio.h>
#include<string.h>
UINT8 SIO_Table[8][3] = { {0,0x2E,0},{ 0,0x2E,1},{ 0,0x4E,0 },{ 0,0x4E,1},{ 1,0x2E,0 },{ 1,0x2E,1 },{ 1,0x4E,0 },{ 1,0x4E,1 } };
UINT8 SIO_TYPE = 0;  //0 ite  1  fintek
//bit7 fintek 4e slave   
//bit6 fintek 4e
//bit5 fintek 2e slave
//bit4 fintek 2e
//bit3 ITE 4e slave
//bit2 ITE 4e
//bit1 ITE 2e slave
//bit0 ITE 2e
UINT8 SIO_INDEX=0;
void EnterSIO(UINT8 SIO_TYPE,UINT8 SIO_INDEX,UINT8 slave)
{
	if (SIO_TYPE==0)
	{
		IoWrite8(SIO_INDEX, 0x87);
		IoWrite8(SIO_INDEX, 0x01);
		IoWrite8(SIO_INDEX, 0x55);
		if (SIO_INDEX==0x2E)
		{
			IoWrite8(SIO_INDEX, 0x55);
		}
		else
		{
			IoWrite8(SIO_INDEX, 0xAA);
		}
		if (slave==0)
		{
			IoWrite8(SIO_INDEX, 0x22);
			IoWrite8(SIO_INDEX, 0x80);
		}
		else
		{
			IoWrite8(SIO_INDEX, 0x22);
			IoWrite8(SIO_INDEX, 0x00);
		}
	}
	else if (SIO_TYPE == 1)
	{
		IoWrite8(SIO_INDEX, 0x87);
		IoWrite8(SIO_INDEX, 0x87);
	}
}
void ExitSIO(UINT8 SIO_TYPE, UINT8 SIO_INDEX)
{
	if (SIO_TYPE==0)
	{
		IoWrite8(SIO_INDEX, 0x02);
		IoWrite8(SIO_INDEX+1, 0x02);
		//printf("exit ite IO\n");
	}
	else
	{
		IoWrite8(SIO_INDEX, 0xAA);
		//printf("exit fintek IO\n");
	}
}
void ConfigSIO()
{
	int SIO_LSB = 0, SIO_MSB = 0;
	for (int i = 0; i < 8; i++)
	{
		EnterSIO(SIO_Table[i][0], SIO_Table[i][1], SIO_Table[i][2]);
		if (SIO_Table[i][0]==0)
		{
			IoWrite8(SIO_Table[i][1], 0x20);
			SIO_MSB = IoRead8(SIO_Table[i][1] + 1);
			IoWrite8(SIO_Table[i][1], 0x21);
			SIO_LSB = IoRead8(SIO_Table[i][1] + 1);
			//printf("ITE SIO_LSB=%x  ",SIO_LSB);
			//printf("ITE SIO_MSB=%x\n",SIO_MSB);
			if (SIO_MSB==0x87&& (SIO_LSB!=0xFF))
			{
				SIO_TYPE |= (1 << i);
			}
		}
		else
		{
			IoWrite8(SIO_Table[i][1], 0x20);
			SIO_LSB = IoRead8(SIO_Table[i][1] + 1);
			IoWrite8(SIO_Table[i][1], 0x21);
			SIO_MSB = IoRead8(SIO_Table[i][1] + 1);
			//printf("FINTEK SIO_LSB=%x  ",SIO_LSB);
			//printf("FINTEK SIO_MSB=%x\n",SIO_MSB);
			if (SIO_MSB == 0x01 && (SIO_LSB == 0x01))
			{
				SIO_TYPE |= (1 << i);
			}
		}
		ExitSIO(SIO_Table[i][0], SIO_Table[i][1]);
	}
	//printf("SIO_TYPE=%x",SIO_TYPE);
}
void ReadLDN(UINT8 SIO_TYPE, UINT8 SIO_INDEX, UINT8 slave,UINT8 LDN, FILE *file)
{
	UINT8 index;
	UINT8 data;
	EnterSIO(SIO_TYPE, SIO_INDEX, slave);
	IoWrite8(SIO_INDEX, 0x07);
	IoWrite8(SIO_INDEX+1, LDN);
	fprintf(file, "LDN=0x%X\n\n",LDN);
	//fprintf(file, "00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F\n");
	for (UINT8 i = 0; i < 0x10; i++)
	{

		//fprintf(file, "%02x", i);
		for (UINT8 j = 0; j < 0x10; j++)
		{
			index = j + (i << 4);
			IoWrite8(SIO_INDEX, index);
			data = IoRead8(SIO_INDEX + 1);
			fprintf(file, "  %02x",data);
		}
		fprintf(file, "\n\n");
	}
	ExitSIO(SIO_TYPE, SIO_INDEX);
}
void ReadEC(UINT8 SIO_TYPE, UINT8 SIO_INDEX, UINT8 slave,FILE *file)
{
	UINT8 index;
	UINT8 data;
	UINT8 EC_LSB = 0, EC_MSB = 0;
	UINT16 EC_ADDR=0;
    EnterSIO(SIO_TYPE, SIO_INDEX, slave);
	IoWrite8(SIO_INDEX, 0x07);
	IoWrite8(SIO_INDEX+1, 0X04);
    IoWrite8(SIO_INDEX, 0x60);
    EC_MSB=IoRead8(SIO_INDEX + 1);
	EC_MSB&=0x0F;
    IoWrite8(SIO_INDEX, 0x61);
    EC_LSB=IoRead8(SIO_INDEX + 1);
    EC_LSB&=0xF0;
	ExitSIO(SIO_TYPE, SIO_INDEX);
	EC_ADDR=(EC_MSB<<8)+EC_LSB;
	fprintf(file, "EC index is %x\n",EC_ADDR+5);
	for (UINT8 i = 0; i < 0x10; i++)
	{
		//fprintf(file, "%02x", i);
		for (UINT8 j = 0; j < 0x10; j++)
		{
			index = j + (i << 4);
			IoWrite8(EC_ADDR+0x05, index);
			data = IoRead8(SIO_INDEX + 0x06);
			fprintf(file, "  %02x",data);
		}
		fprintf(file, "\n\n");
	}

}
void PrintHelp()
{
	printf("usage:readsio.efi [filename]\n");
	printf("example:readsio.efi sio.txt\n");
	printf("if no filename,the default is \"sio.txt\"\n");
}
int main(int argc,char *argv[])
{
	int LDNNumber = 0;
	UINT8 SIO_LSB = 0, SIO_MSB = 0;
	char *filename;
	FILE *file;
	if((argc>2))
	{
		printf("parameter is  too much\n");
		PrintHelp();
		return 1;
	}
	else if (argc<2)
	{
		strcpy(filename,"sio.txt");
		PrintHelp();
	}
	else
	{
        strcpy(filename,argv[1]);
	}	
	file=fopen(filename, "w+");
	if (file!=NULL)
	{
		//printf("Create file successed!\n");
	}
	else{
		printf("Create file failed!\n");
	}
	ConfigSIO();
	for (int i = 0; i < 8; i++)
	{
		if (SIO_TYPE&(1<<i))
		{
			EnterSIO(SIO_Table[i][0], SIO_Table[i][1], SIO_Table[i][2]);
			if (SIO_Table[i][0]==0)
			{
				LDNNumber = 0x0C;
				IoWrite8(SIO_Table[i][1], 0x20);
				SIO_LSB = IoRead8(SIO_Table[i][1] + 1);
				IoWrite8(SIO_Table[i][1], 0x21);
				SIO_MSB = IoRead8(SIO_Table[i][1] + 1);
				if(SIO_Table[i][2]==0)
				{
					if (SIO_Table[i][1]==0x2E)
					{
						fprintf(file, "ITE IO %02x%02x, adress=0x%x\n", SIO_MSB, SIO_LSB,SIO_Table[i][1]);
					}
					else
					{
						fprintf(file, "ITE IO %02x%02x, adress=0x%x\n", SIO_MSB, SIO_LSB,SIO_Table[i][1]);
					}
				}
				else{
					if (SIO_Table[i][1]==0x2E)
					{
						fprintf(file, "ITE IO %02x%02x slav, adress=0x%x\n", SIO_MSB, SIO_LSB,SIO_Table[i][1]);
					}
					else
					{
						fprintf(file, "ITE IO %02x%02x slav, adress=0x%x\n", SIO_MSB, SIO_LSB,SIO_Table[i][1]);
					}
				    }
				
			}
			else
			{
				LDNNumber = 0x15;
				if(SIO_Table[i][2]==0)
				{
					fprintf(file, "Fintek IO\n");
				}
				else{
                    fprintf(file, "Fintek IO slave\n");
				}
				
			}
			ExitSIO(SIO_Table[i][0], SIO_Table[i][1]);
			for (UINT8 j = 0; j < LDNNumber+1; j++)
			{
				ReadLDN(SIO_Table[i][0], SIO_Table[i][1], SIO_Table[i][2], j, file);
			}
			ReadEC(SIO_Table[i][0], SIO_Table[i][1], SIO_Table[i][2],file);
		}
	}
	fclose(file);
    return 0;
}
