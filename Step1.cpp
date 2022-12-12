

#include <bits/stdc++.h>
using namespace std;

void step1(string PATTERN, int* WITNESS, int n){
    for(int i=1; i<n; i++){
        // shift = i;
        int j = 0, k = i;
        int count = 1;
        while(k<n){
            if(PATTERN[j] == PATTERN[k]){
                count++, j++, k++;
                continue;
            }else{
                WITNESS[i] = count; // first mismatch
                break;
            }
        }
    }
}
int main(){
    string PATTERN;
    cin >> PATTERN;
    int n = PATTERN.size();
    int WITNESS[n] = {0};
 
    step1(PATTERN, WITNESS, n);

    for(int i=0; i<n; i++)
    {
        cout << "[" << PATTERN[i] << "] ";
    }

    cout << endl;

    for(int i=0; i<n; i++)
    {
        cout << "[" << WITNESS[i] << "] ";
    }





    return 0;
}
