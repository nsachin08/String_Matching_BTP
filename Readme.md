
# Parallel Pattern Matching

A Parallel Pattern Matching String Algorithm for finding a long size Pattern in Large-size text file using Parallel Programming.

Text Size can be upto GBs.(Experimented upto 100GB).
Pattern Size can be upto few MBs.(Experimented upto 500MB).



## Reference

 - [Optimal Parallel Pattern Matching in String](http://users.umiacs.umd.edu/~vishkin/TEACHING/ENEE651S14/SLIDES/V85-witness.pdf)



## Files :

#### Input File

```http
  pattern.txt -> Contains Pattern to be searched.
  t1.txt -> Contains Text where Pattern is to be found.
```




## Run

To run this code :

```bash
  g++ -o main1 -fopenmp main1.cpp
```


