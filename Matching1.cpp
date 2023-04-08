/* g++ -o Matching1 -fopenmp Matching1.cpp */

#include<bits/stdc++.h>
#include<omp.h>
using namespace std;

#define SIZE_BUF1 8192 // reads from file .
#define SIZE_BUF2 16384 // BUF1  stores in BUF2 .
#define SIZE_BUF3 16384 // BUF2 copy into BUF3 and search operation runs in BUF3 .
#define half_junction 8192
#define pb_size 650 // pattern bucket size
#define SIZE_pattern 2064

int n , m;
int mxk;



char BUFFER1[SIZE_BUF1+1];
char BUFFER2[SIZE_BUF2+1];
char BUFFER3[SIZE_BUF3+2];
char PATTERN[SIZE_pattern];
bool buf2_full = false;
bool buf3_full = false;
bool read_complete = false;
int Left[17][100000];

const int hjs = 8192;
const int js = 16384;

char JUNCTION[js+1]; // JUNCTION SIZE;
char P_JUNCTION[hjs+1]; // Prefix Junction
char S_JUNCTION[hjs+1]; // suffix junction
char TP_JUNCTION[hjs+1]; // Temporary prefix junction ;

bool jflag =false;

int WITNESS[SIZE_pattern] ;


bool MATCH[16500]; 

vector<int> Index;
int text_counter =0;


int cnt = 0;


//Left Find -> FInd lowest index in witnes[1..2^k] such that Witness[i] = 0;
int Lt(int k , int a)
{	
    if(k == 0)
    {
        Left[k][a] = a;
        return Left[k][a]=a;
    }   

	int start = pow(2,k) * (a-1) +1 ; // K=1 ,A =1 , start = 1 ,end=2   
	int end = pow(2,k)*(a);
	for(int i = start; i <= min(end,n); i++)
	{
		if(MATCH[i] == true )
		{		
			return i;
		}
	}

	return 0;
}

int Witness_Generator(string s , int offset) // Generates Witness;
{       
    // cout<<"Witness Generator Called "<<endl;
    int ans=0;
    bool flag = false;

    #pragma omp parallel
    {      
        int local =-1;

        #pragma omp for
        for(int j=0;j<s.size()-offset;j++)
        {
            if(s[j] != s[j+offset])
            {
                local = j;
                flag = true;
            }
        }

        #pragma omp critical
        {
            if(flag && local >= 0 && local< ans)
            {
                ans = local;
            }
        }
    }

    if(flag)
    {
        return ans+1;
    }
    else{
        return 0;
    }
}


void BOX3(int k,int last) // subpart of Step 2;
{   
    // cout<<k<<" "<<last << endl;
    #pragma omp parallel
    {
        #pragma omp for 
            for(int a=1; a<= last;a++)
            {	
                int ltk = Lt(k-1,2*a-1) ;
                int rtk = Lt(k-1,2*a);
                if(ltk == 0)
                {	
                    Left[k][a]=rtk;
                }
                else if (rtk == 0)
                {	
                    Left[k][a] = ltk;
                }
                else{	
                        
                        int j1 = Lt(k-1, 2*a);  //j1 //rtk
                        int j2 = Lt(k-1, 2*a-1); //j2 // ltk
                        int w = WITNESS[j1-j2+1];
                        char x = PATTERN[w];
                        char y = PATTERN[j1-j2+w];
                        char z = BUFFER3[j1-1+w];


                        if(z != y)
                        {
                            MATCH[j2] = false;
                        }

                        if( x != z)
                        {
                            MATCH[j1] = false;
                        }

                        if(MATCH[j2] == true)
                        {
                            Left[k][a] = j2;
                        }
                        else if(MATCH[j1] == true)
                        {
                            Left[k][a]=j1;
                        }
                        else{
                            Left[k][a] = 0;
                        }
                }
        }
    }
}



void step3() // marking MATHC[i] = false; 
{

    int num = (n-m+1);
    int denom = pow(2,floor(log2(m))-1);
    int end = num/denom;
    if(num%denom)
    {
        end ++;
    }

     for(int kk=1;kk<=n-m+1;kk++)
     {  
        
        if(MATCH[kk])
        {
            #pragma omp parallel
            {
                #pragma omp for 
                    for(int j=1;j<=end;j++)
                    {
                        #pragma omp parallel
                        {
                            #pragma omp for
                                for(int i=1 ; i<=m; i++)
                                {
                                    
                                    int lgm = floor(log2(m))-1;
                                    int tj=Left[lgm][j];
                                    
                                    if(tj != 0)
                                    {
                                        
                                        if(BUFFER3[tj+i-1] != PATTERN[i])
                                        {
                                            MATCH[tj]=false;
                                        }
                                    }
                                }
                        }
                    }
            }    
        }
     }

}

void Runner(int tcounter) // after creating witnes array , this get called.
{
    n = strlen(BUFFER3)-1;
    m = strlen(PATTERN)-1;
    // printf("%d ->n , %d -> m",n,m);
    memset(Left,0,sizeof(Left));
    memset(MATCH,0,sizeof(MATCH));
    // printf("%d - diff \n",n-m+1);
    #pragma omp parallel
	{
		#pragma omp for 
		for(int i=1;i<=n-m+1;i++)
		{
			MATCH[i] = true;
		}
	}

	for(int k=1;k<=mxk;k++)
	{   
        int last = (n-m+1)/(1<<k);
        if((n-m+1) % (1<<k))
        {
            last++;
        }

		BOX3(k,last);

	}


    step3();
      

    int count=0;

    for(int i=1;i<=n-m+1;i++)
    {   
        
        if(MATCH[i])
        {       
            // printf("%d = i %d = reading \n",i ,tcounter);
            count++;
            // printf("size = %d  text_counter =%d \n",SIZE_BUF3,text_counter);
            int pt = (SIZE_BUF3 * tcounter );

            if(jflag)
            {
                // printf("%d = text counter from runner \n",tcounter);
                pt = pt-hjs ; /* ================================================ depends on pattern bucket size  =======================================*/
            }

            // printf("MATCH[%d]:= %d \n",i+(pt),MATCH[i]);
            // printf("i = %d , pt = %d , counter =%d  SIZE_BUF3 = %d \n",i,pt,text_counter,SIZE_BUF3);
            Index.push_back(i+pt);
            // cout<<Index.back()<<endl;
            // printf("Found = %d \n",Index.back());
            // cout<<pt<<"\n";
        }
    }
    // printf("I am runnner \n");
}




/*==========================================================================*/


void rw()
{

    PATTERN[0]='#';

    // Read Pattern in PATTERN ARRAY;
    fstream P_Read;
    P_Read.open("ip.txt",ios::in);
    P_Read.read((char *)&PATTERN[1],pb_size);  /* =========================================== depends on pattern bucket size ===================*/

    // printf("%s\n",(char *)&PATTERN[1]);

    memset(WITNESS,0,sizeof(WITNESS));
    #pragma omp parallel
    {
        #pragma omp for
        for(int i=1;i<=(SIZE_pattern/2)+1;i++)
        {
            WITNESS[i+1] = Witness_Generator(&PATTERN[1],i);
        }
    }

    // for(int i=0;i<=(pb_size/2)+1; i++ )
    // {
    //     printf("Witness[%d] : %d \n",i,WITNESS[i]);
    // }    


    m= strlen(PATTERN) -1;
    mxk = log2(m)-1;

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if(tid == 0) // reader  
        {       
            ifstream T_Read;
            T_Read.open("it1.txt",ios::in);
            memset(BUFFER2,0,sizeof(BUFFER2));
            
            while((buf2_full == false) && T_Read.read((char *)&BUFFER1,SIZE_BUF1)) // if buffer1 haven't read  the given size limit it won't go inside the loop;
            {   
                strcat(BUFFER2,BUFFER1);
                memset(BUFFER1,0,sizeof(BUFFER1));
                while(strlen(BUFFER2) != SIZE_BUF2 && T_Read.read((char *)&BUFFER1,SIZE_BUF1)  ) // Read only given size;
                {   
                    strcat(BUFFER2,BUFFER1);
                    memset(BUFFER1,0,sizeof(BUFFER1));
                }
                if(strlen(BUFFER1) )
                {
                    strcat(BUFFER2,BUFFER1);
                }


                buf2_full = true;
                while(buf2_full); // wait till the buf2 is full;
                // BUFFER 2 empty
                // printf("%s",BUFFER2);
                memset(BUFFER2,0,sizeof(BUFFER2));
                memset(BUFFER1,0,sizeof(BUFFER1)); // BUFFER 1 empty
            }
           
            if(strlen(BUFFER1)> 0)
            {    
                while(buf2_full);
                strcat(BUFFER2,BUFFER1);
                // cout<<BUFFER2;
                buf2_full=true;
                memset(BUFFER1,0,sizeof(BUFFER1));
            }
            read_complete =true;
        }

        if(tid == 1) // consumer
        {       
            memset(BUFFER3,0,sizeof(BUFFER3));
            memset(P_JUNCTION,0,sizeof(P_JUNCTION));
            int k=0;
            while(read_complete == false)
            {   
                k++;
                while(buf2_full == false && read_complete == false);
                while(buf3_full);
                BUFFER3[0]='#';
                strcat(BUFFER3,BUFFER2);
                buf3_full = true;
                buf2_full = false;
                memset(BUFFER2,0,sizeof(BUFFER2));
                // cout<<BUFFER3;


                  Runner(text_counter); // search 


                 // Junction handling

                 /*
                    bUFFER3.

                    PJUNCTION = Prefix of junction; // Extracted suffix from previous buffer which becomes prefix of junction;
                    SJUNCTION =Suffix of Junction; // Extract prefix of current buffer which becomes suffix of junction

                    JUNCTION = PJUNCTION + SJUNCTION;

                    TPJUNCTION =  extract suffix of current buffer which becomes prefix for next iteration;

                 */

                memset(S_JUNCTION,'\0',sizeof(S_JUNCTION));
                // printf("%ld = size of before S junction \n",strlen(S_JUNCTION));
                strncat(S_JUNCTION,&BUFFER3[1],hjs);    
                // printf("%ld = size of after S junction \n",strlen(S_JUNCTION));  


                memset(TP_JUNCTION,0,sizeof(TP_JUNCTION));
                strncat(TP_JUNCTION,&BUFFER3[strlen(BUFFER3)-hjs],hjs);  

                // printf("BUFFER : %ld\n\n",strlen(BUFFER3));
                // printf("PJUNCTION : %ld\n\n",strlen(P_JUNCTION));
                // printf("SJUNCTION : %ld\n\n",strlen(S_JUNCTION));
                // printf("TPJUNCTION : %ld\n\n",strlen(TP_JUNCTION));

                // printf("JUNCTION : %s%s\n\n",P_JUNCTION,S_JUNCTION);

                memset(BUFFER3,0,sizeof(BUFFER3));
                
                BUFFER3[0]='#';

                //  printf("%d = text counter from thread 2 consumer times : %d \n \n",text_counter,k);

                // printf("%ld - length Buffer 3 %ld - length of Pjunction %ld sjunction \n",strlen(BUFFER3),strlen(P_JUNCTION),strlen(S_JUNCTION));

                strncat(BUFFER3,P_JUNCTION,strlen(P_JUNCTION));
                strncat(BUFFER3,S_JUNCTION,strlen(S_JUNCTION));

                //  printf("%ld - length Buffer 3 %ld - length of Pjunction %ld sjunction \n",strlen(BUFFER3),strlen(P_JUNCTION),strlen(S_JUNCTION));
                
                if(strlen(P_JUNCTION) !=0 )
                {   
                    
                    jflag=1;
                    //   printf("2 =========== \n");
                    //   printf("%d = text counter from thread 2 inside jflag \n",text_counter);

                    // printf("%s \n",BUFFER3);

                     Runner(text_counter); // search 
                    //   printf("2 =========== \n");
                    // cout<<BUFFER3;
                    jflag=0;
                }
                memset(P_JUNCTION,0,sizeof(P_JUNCTION));
                // printf("%ld length of tp junction \n",strlen(TP_JUNCTION));
                strncat(P_JUNCTION,TP_JUNCTION,strlen(TP_JUNCTION));   
                // printf("%ld length of p junction at end \n",strlen(P_JUNCTION));             

               

                text_counter ++;


                memset(JUNCTION,0,sizeof(JUNCTION));

                /*   ---------------------------    */
                buf3_full = false;
                memset(BUFFER3,0,sizeof(BUFFER3));
            }
        }
    }
    return ;
}


void validate() // check for all generated index from pattern[0] .. and keep eliminating the unecessat index;.
{   
    // printf("In validate \n");
    fstream T_read;
    fstream P_Read;
   
    P_Read.open("ip.txt",ios::in);
    int space = pb_size;  //limit; //see where you are reading for PATTERN chunk number 1 ;
    char B_PATTERN[space+1];
    memset(B_PATTERN,0,sizeof(B_PATTERN));
   
    P_Read.seekg(space);
    
    char para[space+1];
    
    // cout<<endl;

    int j=0; // multiply pattern
    //  P_Read.read((char *)&B_PATTERN[0],sizeof(B_PATTERN));
    //  printf("%s\n",B_PATTERN);

    while(P_Read.read((char *)&B_PATTERN[0],space))  
    {    
        // printf("%s -read\n",B_PATTERN);
        T_read.open("it1.txt",ios::in);
        j++;
        vector<int> Temp;
        for(int i=0;i<Index.size();i++)
        {   
            // printf("\n");
            memset(para,0,sizeof(para));   
            T_read.seekg(Index[i]+j*(space) -1 );
            T_read.read((char*)&para,space);
            // printf("B pattern:%s\n\n",B_PATTERN);
            // printf("para pattern:%s\n\n",para);

            if(strcmp(B_PATTERN,para) == 0)
            {   
                // printf("Pass \n");
                Temp.push_back(Index[i]);
            }   
        }
        Index = Temp;
        memset(B_PATTERN,0,sizeof(B_PATTERN));
        memset(para,0,sizeof(para));
        T_read.close();

    }

    if(strlen(B_PATTERN) != 0)
    {   
        // printf("%s -last read",B_PATTERN);
        T_read.open("it1.txt",ios::in);
        j++;
        vector<int> Temp;
        for(int i=0;i<Index.size();i++)
        {
            memset(para,0,sizeof(para));   
            T_read.seekg(Index[i]+(j*space)-1);
            T_read.read((char*)&para,strlen(B_PATTERN));

            // printf("B pattern:%s\n\n",B_PATTERN);
            // printf("para pattern:%s\n\n",para);

            if(strcmp(B_PATTERN,para) == 0)
            {   
                // printf("pass for %d last \n",Index[i]);
                Temp.push_back(Index[i]);
            } 
            // else{
            //     printf("Fail for %d \n",Index[i]);
            // }  
        }
        Index = Temp;
        T_read.close();
    }



    cnt = Index.size();
        
}


int main()
{   
    omp_set_num_threads(100);

    double itime, ftime, exec_time;
    itime = omp_get_wtime();

    rw(); // generate all found for pattern[0] . when pattern is broken in multiple chunks ,. pattern[0] , pattern[1]... pattern[n];
    printf("\nTotal Count :%ld before validate \n",Index.size());
    validate(); 

    // printf("\n");
    printf("\nTotal Count :%ld after validate \n",Index.size());
   
    ftime = omp_get_wtime();
    exec_time = ftime - itime;
    printf("\n\nTime taken is %f \n", exec_time);

   
    set<long long int> S; // to remove duplicate entries from Vector;
    for(int i=0;i<Index.size();i++)
    {
        S.insert(Index[i]);
    }
    // printf("Here Size of Vector is :%ld\n",Index.size());
    printf("Actual Match Found : %ld\n",S.size());
    // for(auto s: S)
    // {
    //     cout<<"Location : "<<s<<" "<<endl;
    // }

    return 0;
}
