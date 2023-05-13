
/* g++ -o main1 -fopenmp main1.cpp */

// pattern = 2mb
// text = 20gb


#include<bits/stdc++.h>
#include<omp.h>
using namespace std;

#define SIZE_BUF2 256*1024    
#define SIZE_BUF3 256*1024   
#define half_junction 128*1024 
#define pb_size 128*1024  
#define SIZE_pattern 65*1024

const long long int hjs = 128*1024 ;
const long long int js = 256*1024;

long long int n , m;
long long int mxk;

string BUFFER1;
string BUFFER2;
string BUFFER3;
string PATTERN;

bool buf2_full = false;
bool buf3_full = false;
bool read_complete = false;
long long int Left[20][100000];

string JUNCTION; // JUNCTION SIZE;
string P_JUNCTION; // Prefix Junction
string S_JUNCTION; // suffix junction
string TP_JUNCTION; // Temporary prefix junction ;

bool jflag =false;

long long int WITNESS[SIZE_pattern] ;

bool MATCH[16500]; 

vector<long long int> Index;
long long int text_counter =0;


long long int cnt = 0;


//Left Find -> FInd lowest index in witnes[1..2^k] such that Witness[i] = 0;
long long int Lt(long long int k , long long int a, bool MATCH[])
{
    if(k == 0)
    {
        Left[k][a] = a;
        return Left[k][a]=a;
    }   

	long long int start = pow(2,k) * (a-1) +1 ; // K=1 ,A =1 , start = 1 ,end=2   
	long long int end = pow(2,k)*(a);
	for(long long int i = start; i <= min(end,n); i++)
	{
		if(MATCH[i] == true )
		{		
			return i;
		}
	}
	return 0;

}

long long int Witness_Generator(string s , long long int offset) // Generates Witness;
{
    long long sz = s.size()-offset;
    long long arr[sz];

    #pragma omp parallel
    {
        #pragma omp for
        for(long long i=0;i<sz;i++)
        {
            arr[i] = INT_MAX;
        }
    }

    #pragma omp parallel
    {
        #pragma omp for
        for(long long  j=0;j<sz;j++)
        {
            if(s[j] != s[j+offset])
            {
                arr[j]=j+1;
            }
        }
    }

    long long n = sz;

    if((n & (n-1)) != 0)
    {
        n = 1<< ((long long int)log2(n)+1);
    }

    long long* t = (long long*) malloc(n*sizeof(n));

    #pragma omp parallel
    {
        #pragma omp for
        for(long long i=0;i<n;i++)
         {
            if(i<sz)
            {
                t[i] =arr[i];
            }
            else
            {
                t[i]= INT_MAX;
            }
         }
    }
    
    long long M[2*n];

    #pragma omp parallel
    {
        #pragma omp for
        for(long long int i=0; i<n*2; i++)
        {
            M[i] = 0;
        }   

    }

    #pragma omp parallel
    {
        #pragma omp for
        for(long long i=0;i<n;i++)
        {
            M[i+n]= t[i];
        }
    }
    for(long long r = 1 ; r<=log2(n);r++)
    {
        long long offset = n >>r;
        #pragma omp parallel
        {
            #pragma omp for
            for(long long i=0;i<offset;i++)
            {
                long long j = i+offset;
                M[j]= min(M[2*j],M[(2*j) +1]);
            }
        }
    }

    if(M[1] == INT_MAX)
    {
        return 0;
    }
    else
    {
        return M[1];
    }
}

void BOX3(long long int k,long long int last,string BUFFER3, bool MATCH[],long long int Left[][100000]) // subpart of Step 2;
{   

    #pragma omp parallel
    {
        #pragma omp for 
            for(long long int a=1; a<= last;a++)
            {	
                long long int ltk = Lt(k-1,2*a-1,MATCH) ;
                long long int rtk = Lt(k-1,2*a,MATCH);
                if(ltk == 0)
                {	
                    Left[k][a]=rtk;
                }
                else if (rtk == 0)
                {	
                    Left[k][a] = ltk;
                }
                else{	
                        
                    long long int j1 = Lt(k-1, 2*a,MATCH);  //j1 //rtk
                    long long int j2 = Lt(k-1, 2*a-1,MATCH); //j2 // ltk
                    long long int w = WITNESS[j1-j2+1];
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
                    else
                    {
                        Left[k][a] = 0;
                    }
                }
        }
    }
}

void step3(string BUFFER3,bool MATCH[],long long int Left[][100000]) // marking MATHC[i] = false; 
{

    long long int num = (n-m+1);
    long long int denom = (1 << ((long long int)(floor(log2(m)))-1));
    long long int end = num/denom;
    if(num%denom)
    {
        end ++;
    }

    for(long long int kk=1;kk<=n-m+1;kk++)
    {  
        if(MATCH[kk])
        {
            #pragma omp parallel
            {
                #pragma omp for 
                for(long long int j=1;j<=end;j++)
                {
                    #pragma omp parallel
                    {
                        #pragma omp for
                        for(long long int i=1 ; i<=m; i++)
                        {
                            long long int lgm = floor(log2(m))-1;
                            long long int tj=Left[lgm][j];
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

void Runner(long long int tcounter, string BUFFER3, bool jflag,bool MATCH[],long long int Left[][100000]) // after creating witnes array , this get called.
{   
    n = BUFFER3.size()-1;
    m = PATTERN.size()-1;

    for(long long int i=0;i<20;i++)
    {
        for(long long int j=0;j<100000;j++)
        {
            Left[i][j] = 0;
        }
    }

    for(long long int i=0;i<16500;i++)
    {
        MATCH[i]= false;
    }

    #pragma omp parallel
	{
		#pragma omp for 
		for(long long int i=1;i<=n-m+1;i++)
		{
			MATCH[i] = true;
		}
	}

	for(long long int k=1;k<=mxk;k++)
	{   
        long long int last = (n-m+1)/(1<<k);
        if((n-m+1) % (1<<k))
        {
            last++;      
        }
		BOX3(k,last,BUFFER3,MATCH,Left);
    }

    step3(BUFFER3,MATCH,Left);

    long long int count=0;
    for(long long int i=1;i<=n-m+1;i++)
    {
        if(MATCH[i])
        {       
            count++;
            long long int pt = (SIZE_BUF3 * tcounter );
            if(jflag)
            {
                pt = pt-hjs ; /* ================================= depends on pattern bucket size =*/
            }
            #pragma omp critical
            {
                Index.push_back(i+pt);
            }
        }
    }
}

void rw()
{
    PATTERN.resize(pb_size+1);

    // Read Pattern in PATTERN ARRAY;
    fstream P_Read;
    P_Read.open("pattern.txt",ios::in);
    PATTERN[0]='#';
    P_Read.read(&PATTERN[1],pb_size);  /* ======================== depends on pattern bucket size =*/

    long long int sz= PATTERN.size()-1;
    memset(WITNESS,0,sizeof(WITNESS));

    #pragma omp parallel
    {
        #pragma omp for
        for(long long int i=1;i<=(sz/2)+1;i++)
        {
            #pragma omp task
            {
                WITNESS[i+1] = Witness_Generator(&PATTERN[1],i);
            }
        }
    }

    m= PATTERN.size() -1;
    mxk = log2(m)-1;
    #pragma omp parallel
    {
        long long int tid = omp_get_thread_num();
        if(tid == 0) // reader  
        {    
            ifstream T_Read;
            T_Read.open("t1.txt",ios::in);
            BUFFER2.clear();
            BUFFER2.resize(SIZE_BUF2);
            
            while(T_Read.read(&BUFFER2[0],SIZE_BUF2)) 
            {   
                BUFFER3.clear();

                BUFFER3.append("#");
                BUFFER3.append(BUFFER2);

                BUFFER2.clear();

                #pragma omp task firstprivate(text_counter, jflag, BUFFER3,MATCH,Left)
                {       
                    Runner(text_counter,BUFFER3,jflag,MATCH,Left);  // 1. Search()
                }
                       
                S_JUNCTION.clear();
                S_JUNCTION.append(BUFFER3,1,hjs);

                TP_JUNCTION.clear();
                TP_JUNCTION.append(BUFFER3,BUFFER3.size()-hjs,hjs);

                BUFFER3.clear();
                BUFFER3.append("#");

                BUFFER3.append(P_JUNCTION,0,P_JUNCTION.size());
                BUFFER3.append(S_JUNCTION,0,S_JUNCTION.size());

                if(P_JUNCTION.size() !=0 )
                {   
                    jflag=1;
                    #pragma omp task firstprivate(text_counter, jflag, BUFFER3,MATCH,Left)
                    {       
                        Runner(text_counter,BUFFER3,jflag,MATCH,Left);   // 2. Search()
                    }
                    jflag=0;
                }

                P_JUNCTION.clear();
                P_JUNCTION.append(TP_JUNCTION,0,TP_JUNCTION.size());    
                TP_JUNCTION.clear();  

                text_counter ++;
                
                BUFFER3.clear();
                BUFFER2.clear();
                BUFFER2.resize(SIZE_BUF2); 
            }
           
            if(T_Read.gcount()>0)
            {
                BUFFER3.clear();
                BUFFER3.append("#");

                BUFFER3.append(BUFFER2);
                BUFFER2.clear();

                if(T_Read.gcount() > PATTERN.size())
                {
                    #pragma omp task firstprivate(text_counter, jflag, BUFFER3,MATCH,Left)
                    {       
                        Runner(text_counter,BUFFER3,jflag,MATCH,Left);  // 3. Search()
                    }
                }
                        
                S_JUNCTION.clear();
                S_JUNCTION.append(BUFFER3,1,hjs);

                TP_JUNCTION.clear();
                TP_JUNCTION.append(BUFFER3,BUFFER3.size()-hjs,hjs);

                BUFFER3.clear();
                BUFFER3.append("#");

                BUFFER3.append(P_JUNCTION,0,P_JUNCTION.size());
                BUFFER3.append(S_JUNCTION,0,S_JUNCTION.size());

                if(P_JUNCTION.size() )
                {   
                    jflag=1;
                    #pragma omp task firstprivate(text_counter, jflag, BUFFER3,MATCH,Left)
                    {       
                        Runner(text_counter,BUFFER3,jflag,MATCH,Left);  // 4. Search()
                    }
                    jflag=0;
                }
            }
        }
        #pragma omp barrier
    }
    return ;
}

void validate() // check for all generated index from pattern[0] .. and keep eliminating the unecessat index;.
{   
    fstream P_Read;
    P_Read.open("pattern.txt",ios::in);
    long long int space = pb_size;//1024 * 200; // pb_size;  //limit; //see where you are reading for PATTERN chunk number 1 ;

    string B_PATTERN;

    P_Read.seekg(space);
    B_PATTERN.clear();
    B_PATTERN.resize(space);
   
    string para;

    long long int j=0; // multiply pattern

    while(P_Read.read(&B_PATTERN[0],space))  
    {    
        j++;
        vector<long long int> Temp;
        #pragma omp parallel
        {
            #pragma omp for private(para)
            for(long long int i=0;i<Index.size();i++)
            {    
                fstream T_read;
                T_read.open("t1.txt",ios::in);

                para.clear(); 
                long long int ml = j*space;
                long long int mv = (Index[i]+ml) -1;
                T_read.seekg(mv);

                long long int sp= space;
                long long int blen=B_PATTERN.size();
                long long int rlen=min(sp,blen);
                para.resize(rlen);
                T_read.read(&para[0],rlen);

                #pragma omp critical
                {
                    if(B_PATTERN == para )
                    {   
                        // printf("Pass \n");
                        Temp.push_back(Index[i]);
                    }  
                }
    
                para.clear();  
                T_read.close();        
            }
        }

        Index = Temp;

        B_PATTERN.clear();
        B_PATTERN.resize(space);
        para.clear();
    }

    para.clear();

    if(P_Read.gcount() > 0)
    {   
        B_PATTERN.resize(P_Read.gcount());
        j++;
        vector<long long int> Temp;
        #pragma omp parallel
        {
            #pragma omp for private(para)
            for(long long int i=0;i<Index.size();i++)
            {    
                fstream T_read;
                T_read.open("t1.txt",ios::in);
                para.clear(); 
                T_read.seekg(Index[i]+j*(space) -1 );
                long long int sp= space;
                long long int blen=B_PATTERN.size();
                long long int rlen=min(sp,blen);
                para.resize(rlen);
                T_read.read(&para[0],rlen);


                #pragma omp critical
                {
                    if(B_PATTERN == para )
                    {   
                    Temp.push_back(Index[i]);
                    }  
                }  
                para.clear();
                T_read.close();
            }
        }
        #pragma omp barrier
        Index = Temp;
    }
    cnt = Index.size();
}

int main()
{   

    omp_set_num_threads(10);
    double itime, ftime, exec_time;
    itime = omp_get_wtime();

    rw(); // generate all found for pattern[0] . when pattern is broken in multiple chunks ,. pattern[0] , pattern[1]... pattern[n];
    printf("\nTotal Count :%ld before validate \n",Index.size());

    validate(); 
     
    printf("\nTotal Count :%ld after validate \n",Index.size());
   
    ftime = omp_get_wtime();
    exec_time = ftime - itime;
    printf("\n\nTime taken is %f \n", exec_time);
   
    set<long long int> S; // to remove duplicate entries from Vector;
    for(long long int i=0;i<Index.size();i++)
    {
        S.insert(Index[i]);
    }
    // printf("Here Size of Vector is :%ld\n",Index.size());
    printf("Actual Match Found : %ld\n",S.size());

    cout << "For Pattern - 2MB & Text - 19 GB & space = 128kb && fb = 256kb & text = 25kb\n";

    return 0;
}