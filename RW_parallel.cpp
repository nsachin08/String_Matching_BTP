#include<bits/stdc++.h>
#include<omp.h>
using namespace std;

#define SIZE_BUF1 10
#define SIZE_BUF2 1000
#define SIZE_BUF3 1000

char BUFFER1[SIZE_BUF1];
char BUFFER2[SIZE_BUF2+1];
char BUFFER3[SIZE_BUF3+1];
bool buf2_full = false;
bool buf3_full = false;
bool read_complete = false;

void empty()
{   
    printf("%s",BUFFER3);
    printf("\n \n \n ============ Consumed ================ \n \n \n");
}


void runnner()
{

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if(tid == 0) // reader  
        {       
            // printf("I am in Thread 1 \n");
            ifstream T_Read;
            T_Read.open("input_text.txt",ios::in);
            memset(BUFFER2,0,sizeof(BUFFER2));
            while((buf2_full == false) && T_Read.read((char *)&BUFFER1,sizeof(BUFFER1))) // if buffer1 haven't read  the given size limit it won't go inside the loop;
            {   
                strcat(BUFFER2,BUFFER1);
                memset(BUFFER1,0,sizeof(BUFFER1));
                while(strlen(BUFFER2) != SIZE_BUF2 && T_Read.read((char *)&BUFFER1,sizeof(BUFFER1))  ) // Read only given size;
                {   
                    strcat(BUFFER2,BUFFER1);
                    memset(BUFFER1,0,sizeof(BUFFER1));
                }
                if(strlen(BUFFER1) )
                {
                    strcat(BUFFER2,BUFFER1);
                }
                // count += strlen(BUFFER2);
                // cout<<BUFFER2<<" "<<"Size of : "<<strlen(BUFFER2)<<endl;
                /* =============       Time to empty the buffer    ===================*/
                buf2_full = true;
                while(buf2_full); // wait till the buf2 is full;
                // BUFFER 2 empty
                memset(BUFFER1,0,sizeof(BUFFER1)); // BUFFER 1 empty
            }
           

            if(strlen(BUFFER1)> 0)
            {    
                while(buf2_full);
                strcat(BUFFER2,BUFFER1);
                buf2_full=true;
            // cout<<BUFFER2<<" "<<"Size of : "<<strlen(BUFFER2)<<endl;
                memset(BUFFER1,0,sizeof(BUFFER1));
            }
            read_complete =true;
            
            // printf("\n All read ...  \n");
        }

        if(tid == 1) // consumer
        {       
            // printf("I am in Thread 2 \n");
            while(read_complete == false)
            {
                while(buf2_full == false && read_complete == false);
                while(buf3_full);
                strcat(BUFFER3,BUFFER2);
                buf3_full = true;
                buf2_full = false;
                memset(BUFFER2,0,sizeof(BUFFER2));
                empty(); // search 
                buf3_full = false;
                memset(BUFFER3,0,sizeof(BUFFER3));
            }

            // printf("\n All consumed ..  \n");
        }
    }

}


int main()
{   
    omp_set_num_threads(2);
    runnner();
    
    return 0;
}
