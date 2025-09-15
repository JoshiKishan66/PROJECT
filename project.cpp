//This Project Based on Hotel Management System
//This Project Facility used Hotel-system(worker)
//start Boop Micro-Project
#include<iostream>
#include<fstream>//for file concept
#include <string.h>

using namespace std;
class Base
{
	protected:
		int ch,chh,n,lop,Pay,room,payment;
		long double Mobile;
		char gen[5];
		string name,adhaar,add;			
		fstream fout;

		
		void set_Choice()
		{
						cout<<"Enter valid Mobile Number +91-->";
						cin>>Mobile;
						fout.open("1.txt",ios::app);
						cout<<"Enter Room Number of Clinte-->";
						cin>>room;
						fout<<"Room Number-->"<<room<<endl;
						cout<<"Enter Name of Client-->";
						getline(cin,name);
						fout<<"Name-->"<<name<<endl;
						cout<<"How many days Book Room?-->";
						fflush(stdin);
						cin>>n;
						fout<<"Book Room-->"<<n<<endl;
						cout<<"Enter Aadhar Number /xxxx-xxxx-xxxx/->";
						fflush(stdin);
						getline(cin,adhaar);
						fout<<"Adhaar Number-->"<<adhaar<<endl;
						cout<<"Enter Address-->";
						fflush(stdin);
						getline(cin,add);
						fout<<"address-->"<<add<<endl;
						cout<<"Enter gender-->";
						fflush(stdin);
						cin>>gen;
						fout<<"Gender-->"<<gen<<endl;		
						cout<<"Your room fees is->"<<(n*1000);
						fout<<"payment-->"<<(n*1500)<<endl;			
						cout<<endl<<"\nHow to pay  this payment\n1.online Payment in mobile-number 2.offline payment on a counter\n-->";
						fflush(stdin);
						cin>>Pay;
						fout<<"-------This Room Detail Is Done.-------"<<endl;
						cout<<endl;
							switch(Pay)
						{
							case 1:
							{
								cout<<"The mobile number of hotel is-+91 9099226684\nPay Room Fees-->";
								cin>>payment;
								cout<<endl;
								cout<<"Payment SucssesFull.!\n"<<endl;
								cout<<endl;
								break;
							}
							case 2:
							{	
								break;				
							}
							default:
							{
								cout<<"Payment Canceld!\n";	
							}	
						}
						cout<<endl;
						cout<<"SucssesFully Booked!"<<endl;		
		}
		void set_choice()
		{
					cout<<"Enter valid Mobile Number +91-->";
						cin>>Mobile;
						fout.open("1.txt",ios::app);
						cout<<"Enter Room Number of Clinte-->";
						cin>>room;
						fout<<"Room Number-->"<<room<<endl;
						cout<<"Enter Name of Client-->";
						fflush(stdin);
						getline(cin,name);
						fout<<"Name-->"<<name<<endl;
						cout<<"How many days Book Room?-->";
						fflush(stdin);
						cin>>n;
						fout<<"Book Room-->"<<n<<endl;
						cout<<"Enter Aadhar Number /xxxx-xxxx-xxxx/->";
						fflush(stdin);
						getline(cin,adhaar);
						fout<<"Adhaar Number-->"<<adhaar<<endl;
						cout<<"Enter Address-->";
						fflush(stdin);
						getline(cin,add);
						fout<<"address-->"<<add<<endl;
						cout<<"Enter gender-->";
						fflush(stdin);
						cin>>gen;
						fout<<"Gender-->"<<gen<<endl;		
						cout<<"Your room fees is->"<<(n*1500);
						fout<<"payment-->"<<(n*1500)<<endl;			
						cout<<endl<<"\nHow to pay  this payment\n1.online Payment in mobile-number 2.offline payment on a counter\n-->";
						fflush(stdin);
						cin>>Pay;
						fout<<"-------This Room Detail Is Done.-------"<<endl;
						cout<<endl;
							switch(Pay)
						{
							case 1:
							{
								cout<<"The mobile number of hotel is-+91 9099226684\nPay Room Fees-->";
								cin>>payment;
								cout<<endl;
								cout<<"Payment SucssesFull.!\n"<<endl;
								cout<<endl;
								break;
							}
							case 2:
							{	
								break;				
							}
							default:
							{
								cout<<"Payment Canceld!\n";	
							}	
						}
						cout<<endl;
						cout<<"SucssesFully Booked!"<<endl;					
		}
		void Function_2()
		{
			string  user,userChoice; 
			
			cout<<"\n1.AC-ROOM or 2.NON-AC Room?";
			fflush(stdin);
			cin>>user;
			cout<<"\n1.Single-Bad or 2.Double-Bad?";
			fflush(stdin);
			cin>>userChoice;
		}
		void set_3()
		{
			int Table,number1;
			cout<<"How Many Mans For Book Table?-->";
			cin>>Table;
			cout<<"\nWhat is Table Number of Client?-->";
			cin>>number1;
			cout<<"\nSucssesfully Book Table!!\n";
		}
	
		
};
class F_Derived
{
	protected:
			int ch,chh,n,lop,Pay,room,payment,PayMent,N_Days;
			long double Mobile;
			char gen[5];
			string name,add,adhaar;
			fstream fout;
			void set_All()
			{				
					cout<<"Enter valid Mobile Number +91-->";
						cin>>Mobile;
						fout.open("1.txt",ios::app);
						cout<<"Enter Room Number of Clinte-->";
						cin>>room;
						fout<<"Room Number-->"<<room<<endl;
						cout<<"Enter Name of Client-->";
						fflush(stdin);
						getline(cin,name);
						fout<<"Name-->"<<name<<endl;
						cout<<"How many days Book Room?-->";
						fflush(stdin);
						cin>>n;
						fout<<"Book Room-->"<<n<<endl;
						cout<<"Enter Aadhar Number /xxxx-xxxx-xxxx/->";
						getline(cin,adhaar);
						fout<<"Adhaar Number-->"<<adhaar<<endl;
						cout<<"Enter Address-->";
						fflush(stdin);
						getline(cin,add);
						fout<<"address-->"<<add<<endl;
						cout<<"Enter gender-->";
						fflush(stdin);
						cin>>gen;
						fout<<"Gender-->"<<gen<<endl;		
						cout<<"Your room fees is->"<<(n*900);
						fout<<"payment-->"<<(n*1500)<<endl;			
						cout<<endl<<"\nHow to pay  this payment\n1.online Payment in mobile-number 2.offline payment on a counter\n-->";
						fflush(stdin);
						cin>>Pay;
						fout<<"-------This Room Detail Is Done.-------"<<endl;
						cout<<endl;
							switch(Pay)
						{
							case 1:
							{
								cout<<"The mobile number of hotel is-+91 9099226684\nPay Room Fees-->";
								cin>>payment;
								cout<<endl;
								cout<<"Payment SucssesFull.!\n"<<endl;
								cout<<endl;
								break;
							}
							case 2:
							{	
								break;				
							}
							default:
							{
								cout<<"Payment Canceld!\n";	
							}	
						}
						cout<<endl;
						cout<<"SucssesFully Booked!"<<endl;								
			}			
					
		void set_all()
		{
				cout<<"Enter valid Mobile Number +91-->";
						cin>>Mobile;
						fout.open("1.txt",ios::app);
						cout<<"Enter Room Number of Clinte-->";
						cin>>room;
						fout<<"Room Number-->"<<room<<endl;
						cout<<"Enter Name of Client-->";
						fflush(stdin);
						getline(cin,name);
						fflush(stdin);
						fout<<"Name-->"<<name<<endl;
						cout<<"How many days Book Room?-->";
						fflush(stdin);
						cin>>n;
						fout<<"Book Room-->"<<n<<endl;
						cout<<"Enter Aadhar Number /xxxx-xxxx-xxxx/->";
						fflush(stdin);
						getline(cin,adhaar);
						fout<<"Adhaar Number-->"<<adhaar<<endl;
						cout<<"Enter Address-->";
						fflush(stdin);
						getline(cin,add);
						fout<<"address-->"<<add<<endl;
						cout<<"Enter gender-->";
						fflush(stdin);
						cin>>gen;
						fout<<"Gender-->"<<gen<<endl;		
						cout<<"Your room fees is->"<<(n*500);
						fout<<"payment-->"<<(n*1500)<<endl;			
						cout<<endl<<"\nHow to pay  this payment\n1.online Payment in mobile-number 2.offline payment on a counter\n-->";
						fflush(stdin);
						cin>>Pay;
						fout<<"-------This Room Detail Is Done.-------"<<endl;
						cout<<endl;
							switch(Pay)
						{
							case 1:
							{
								cout<<"The mobile number of hotel is-+91 9099226684\nPay Room Fees-->";
								cin>>payment;
								cout<<endl;
								cout<<"Payment SucssesFull.!\n"<<endl;
								cout<<endl;
								break;
							}
							case 2:
							{	
								break;				
							}
							default:
							{
								cout<<"Payment Canceld!\n";	
							}	
						}
						cout<<endl;
						cout<<"SucssesFully Booked!"<<endl;		
		}
		void Set_F2()
		{
				cout<<"Enter New-Room Number of Clinte-->";
				fflush(stdin);
				cin>>room;
				cout<<"How many days Book Room ??\n-->";
				fflush(stdin);
				cin>>n;
				
				cout<<endl;
				cout<<"SucssesFully Booked!"<<endl;	
		}	
		Print_set()
		{
			
			ifstream fin;
			fin.open("1.txt",ios::in);
			string tp;
     		while(getline(fin, tp))
		  { 
        	 cout << tp << "\n"; 
    	  }
	fin.close();
		}
			void set_4()
		{
			int Room_number,check_out,PAY,LABLE,PayMent,LABEL;
			string ANY;
			cout<<"What is Room-Number Before Check out?-->";
			cin>>Room_number;
			cout<<"\nBill is Pay or not?\n1.pay2.not-pay\n-->";
			cin>>PAY;
			switch(PAY)
			{
				case 1:
					{
						goto LABLE;		
					}
				case 2:
					{
						LABEL:
						cout<<"\nPay Payment";
						cout<<"\nPay Room-Payment on Counter\n1.Yes or 2.No-->";
						cin>>PayMent;
						if(PayMent==1)
						{
							cout<<"\nOK Room Payment Is Payed.";
							LABLE:
							cout<<"Enter rating for hotel from Client?\n 1-star\n 2-star\n 3-star\n 4-star\n 5-star\n-->";
							fflush(stdin);
							cin>>ANY;
							cout<<"Room is Sucssesfully_check-out.";	
						}
						else if(PayMent==2)
						{
							cout<<"\nPay-First Room PayMent And After Check Out.";
							goto LABEL;
						}
						else
						{
							cout<<"\nSomeThing Error!";	
						}
						break;
					}
			}
		
		}
};
class New:public F_Derived,public Base
{
	protected:
		int chh,ch;
		void set_a()
		{
			lop:
			cout<<"1.Ac Room\n2.Non Ac Room\n-->";
			cin>>chh;
			cout<<endl;
			switch(chh)
			{
				case 1:
					{
						set_All();
						break;
					}
				case 2:
					{
						set_all();
						break;
					}
				default:
					{
						cout<<"Incorect case!\n";
						goto lop;
					}
			}
		}
			void set_b()
		{
			cout<<"1.Ac Room\n2.Non Ac Room\n-->";
			cin>>chh;
			cout<<endl;
			switch(chh)
			{
				case 1:
					{
						set_choice();
						break;
					}
				case 2:
					{
						set_Choice();
						break;
					}
			}
		}
			protected:
		int old;
		long double mobile;
		string NAME;
		
		void set_6()
		{
			exit(0);
		
		}
		void set_2()
		{
				cout<<"Enter old Room-Number-->";
				cin>>old;
				cout<<"Enter Mobile-Number +91-->";
				cin>>mobile;
				cout<<"Enter Any Reson to Change A Room-->";\
				fflush(stdin);
				cin>>NAME;
				Set_F2();//line 133
				Function_2();//line 281
				cout<<"SucssesFully Changed Room.!";
		}	
		void set_1()
		{
			cout<<"1.Single Bad Room\n2.Double Bad Room\n-->";
			cin>>ch;
			cout<<endl;
			switch(ch)
			{
				case 1:
					{
						set_a();
						break;
					}
				case 2:
					{
						set_b();
						break;
					}
			}
		}
};
class Buisness:public New
{
	protected:
		int ch,lable,Main,Mchoice;
		void PutData()
		{
			lable:
			Main:
			cout<<"1.Book A Room\n2.Change A Room\n3.Book Table In Resturant\n4.Check Out From Hotel\n5.See Recorde Of Rooms\n6.Exit\n-->";
			cin>>ch;
			switch(ch)
			{
				case 1:
					{
						set_1();//line 420
						break;
					}
				case 2:
					{
						set_2();//line 407
						break;
				 	}
				case 3:
					{
						set_3();//line 143
						break;	
					}
				case 4:
					{
						set_4();//line 305
						break;
					}
				case 5:
					{
						Print_set();//line 293
						break;
					}
					case 6:
					{
						set_6();//line 402
						break;
					}
				default:
						cout<<"--------------------------------------"<<endl;
						cout<<"Invalid Choice Please Re-Enter Choice\n";
						cout<<"--------------------------------------"<<endl;
						goto lable;
			}
			cout<<"\nEnter 1.For Continue 2. For End -->";
			cin>>Mchoice;
			switch(Mchoice)
			{
				case 1:
					{
						goto Main;
						break;
					}
				case 2:
					{
						break;
					}
			}	
		}
};
class author:public Buisness//author derived from Buisness
{
		protected:
			int n,user,loop;
			void get()
			{
				cout<<"enter  valid id number-->";
				cin>>user;
			}
			public:
				author()
				{
					loop:
					get();//line-355
					n=9636;
					if(user==n)
					{
						cout<<"----------Welcome user--------"<<endl;
						PutData();
					}
					else
					{
						cout<<"---------------------------"<<endl;
						cout<<"Incorect user Id!! Re-Enter"<<endl;
						cout<<"---------------------------"<<endl;
						goto loop;
					}
			}	
			~author()
			{
				cout<<endl<<"---------------------------"<<endl;
				cout<<"Sucssesfully work is done.\n"<<endl;
				cout<<"---------------------------"<<endl;
			}
};
  int main()
  {		
	author object1;
  	return 0;
  }
