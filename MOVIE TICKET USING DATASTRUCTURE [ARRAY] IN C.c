#include<stdio.h>
//Global Variables
int arr[10][10],book[100];
int arr2[10][10],book2[100];
int arr3[10][10],book3[100];
int arr4[10][10],book4[100];
int choice;

//Declaration
void Show_Time();
void Show_Time1();
void Show_Time2();
void Show_Time3();
void Show_Time4();
void SEE_MOVIE_THEATER();


void SEE_MOVIE_THEATER()
{
	int no=1;
	int i,j;
	printf("Total Seats	 In Movie Theater Is 100.\n");
	printf("\n                                             ---------------SCREEN---------------");
	printf("\n----------------------------------------------------------------------------------------------------------------------------------------\n");
	for(i=0;i<10;i++)
	{
		for(j=0;j<10;j++)
		{
			if(no%10==1)
			{
				printf("\n");
			}
			if(no<10)
			{
				printf("|Seat:%d  |    ",no);
				no++;
			}
			if(no>=10&&no<100)
			{
				printf("|Seat:%d |    ",no);
				no++;
			}
			if(no==100)
			{
				printf("|Seat:%d|",no);
				no++;
			}
		}
	} 
	printf("----------------------------------------------------------------------------------------------------------------------------------------\n");
}


void Show_Time1()
{
	int i,j,k=0;
	int count=1,num=1;
	int n,s;
	
	printf("Enter number of tickets you want to book: ");
	scanf("%d",&n);
	if(n>=1 && n<=100)
	{
        for(k=0;k<n;k++)
	    {
			printf("\nEnter the seat Number you want to book in ascending order:");
			scanf("%d",&s);
			for(i=0;i<=100;i++)
			{
				if(book[i]==s)
				{
					printf("Seat  Number:%d Is already booked So Can't Book Again...\n",s);
					return;	
				}
			}
			book[k]=s;
		}
	}
	else
	{
		printf("Not Available...");
	}
	while(num!=101)
	{
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				arr[i][j]=num;
				num++;
			}
		}
	}
		k=0;
		count=1;
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr[i][j]==book[k])
				{
					arr[i][j]=0;
					k++;
				}		
				else
				{
					arr[i][j]=count;
				}
				count++;
			}
		}
		for(k=0;k<n;k++)
		{
			printf("You Select Tickets Is:%d\n",book[k]);
		}
		printf("\n");
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr[i][j]==0)
				{
					printf("[Seat:Book]\t");
				}
				else
				{
					printf("[Seat:%d]\t",arr[i][j]);
				}
			}
			printf("\n");
		}
		
		printf("\nTicket SuccsessFully Booked...\n");
}
void Show_Time2()
{
	int i,j,k=0;
	int count=1,num=1;
	int n,s;
	
	printf("Enter number of tickets you want to book: ");
	scanf("%d",&n);
	if(n>=1 && n<=100)
	{
        for(k=0;k<n;k++)
	    {
			printf("\nEnter the seat Number you want to book in ascending order:");
			scanf("%d",&s);
			for(i=0;i<=100;i++)
			{
				if(book2[i]==s)
				{
					printf("Seat  Number:%d Is already booked So Can't Book Again...\n",s);
					return;	
				}
			}
			book2[k]=s;
		}
	}
	else
	{
		printf("Not Available...");
	}
	while(num!=101)
	{
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				arr2[i][j]=num;
				num++;
			}
		}
	}
		k=0;
		count=1;
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr2[i][j]==book2[k])
				{
					arr2[i][j]=0;
					k++;
				}		
				else
				{
					arr2[i][j]=count;
				}
				count++;
			}
		}
		for(k=0;k<n;k++)
		{
			printf("You Select Tickets Is:%d\n",book2[k]);
		}
		printf("\n");
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr2[i][j]==0)
				{
					printf("[Seat:Book]\t");
				}
				else
				{
					printf("[Seat:%d]\t",arr2[i][j]);
				}
			}
			printf("\n");
		}
		
		printf("\nTicket SuccsessFully Booked...\n");
}
void Show_Time3()
{
	int i,j,k=0;
	int count=1,num=1;
	int n,s;
	
	printf("Enter number of tickets you want to book: ");
	scanf("%d",&n);
	if(n>=1 && n<=100)
	{
        for(k=0;k<n;k++)
	    {
			printf("\nEnter the seat Number you want to book in ascending order:");
			scanf("%d",&s);
			for(i=0;i<=100;i++)
			{
				if(book3[i]==s)
				{
					printf("Seat  Number:%d Is already booked So Can't Book Again...\n",s);
					return;	
				}
			}
			book3[k]=s;
		}
	}
	else
	{
		printf("Not Available");
	}
	while(num!=101)
	{
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				arr3[i][j]=num;
				num++;
			}
		}
	}
		k=0;
		count=1;
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr3[i][j]==book3[k])
				{
					arr3[i][j]=0;
					k++;
				}		
				else
				{
					arr3[i][j]=count;
				}
				count++;
			}
		}
		for(k=0;k<n;k++)
		{
			printf("You Select Tickets Is:%d\n",book3[k]);
		}
		printf("\n");
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr3[i][j]==0)
				{
					printf("[Seat:Book]\t");
				}
				else
				{
					printf("[Seat:%d]\t",arr3[i][j]);
				}
			}
			printf("\n");
		}
		
		printf("\nTicket SuccsessFully Booked...\n");
}
void Show_Time4()
{
	int i,j,k=0;
	int count=1,num=1;
	int n,s;
	
	printf("Enter number of tickets you want to book: ");
	scanf("%d",&n);
	if(n>=1 && n<=100)
	{
        for(k=0;k<n;k++)
	    {
			printf("\nEnter the seat Number you want to book in ascending order:");
			scanf("%d",&s);
			for(i=0;i<=100;i++)
			{
				if(book4[i]==s)
				{
					printf("Seat  Number:%d Is already booked So Can't Book Again...\n",s);
					return;	
				}
			}
			book4[k]=s;
		}
	}
	else
	{
		printf("Not Available...");
	}
	while(num!=101)
	{
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				arr4[i][j]=num;
				num++;
			}
		}
	}
		k=0;
		count=1;
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr4[i][j]==book4[k])
				{
					arr4[i][j]=0;
					k++;
				}		
				else
				{
					arr4[i][j]=count;
				}
				count++;
			}
		}
		for(k=0;k<n;k++)
		{
			printf("You Select Tickets Is:%d\n",book4[k]);
		}
		printf("\n");
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				if(arr4[i][j]==0)
				{
					printf("[Seat:Book]\t");
				}
				else
				{
					printf("[Seat:%d]\t",arr4[i][j]);
				}
			}
			printf("\n");
		}
		
		printf("\nTicket SuccsessFully Booked...\n");
}
void See_Show_Time()
{
	printf("\nFirst Show Time-->10:00 AM To 01:00 PM\nSecond Show Time-->03:00 PM To 06:00 PM\nThird Show Time-->06:00 PM To 09:00 PM\nFourth Show Time-->09:00 PM To 12:00 AM\n:");

}
void Show_Time()
{
	printf("\n1.First Show Time-->10:00 AM To 01:00 PM\n2.Second Show Time-->03:00 PM To 06:00 PM\n3.Third Show Time-->06:00 PM To 09:00 PM\n4.Fourth Show Time-->09:00 PM To 12:00 AM\n:");
	scanf("%d",&choice);
	switch(choice)
	{
		case 1:	Show_Time1();
				break;
		case 2:	Show_Time2();
				break;
		case 3: Show_Time3();
				break;
		case 4: Show_Time4();
				break;
		default:
				printf("Invalid Choice...\n");
				break;
	}
		
}

int main()
{
	SEE_MOVIE_THEATER();
	int choice;	
	do
	{
	printf("\n1.See Show-Time\n2.Book_Movie-Ticket\n3.Exit\nEnter Your Choice:");
	scanf("%d",&choice);
	printf("\n\n");
	switch(choice)
	{
		case 1:
				See_Show_Time();
				break;
		case 2:
				Show_Time();
				break;
		case 3:
				break;
		default:
				printf("Please Enter Valid Choice...");
	}
	}while(choice<3);
	return 0;	
}
