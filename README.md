# cache_simulator
cache simulator with C language

<br>

## How to make trace file
```
make 'sample.trc' file in same directory with C file

- trace file format : <Address> <R/W> <data>

ex)
22292218 W 32
2229221C W 31
00010000 R
00010004 R
00010008 R

```
<br>

## Compile
```
gcc cache_sim.c -o cache_sim  
./cache_sim -a=<cache size> -s=<block size> -b=<set size>
```
<br>

## Cache Policy
- Write Policy : Write-back
- The size of the cache, block and set must be an exponential multiplier of 2.

<br>

## Result Example
```
0: 00001571 0000156f v:1 d:0 
   00001571 0000156f v:1 d:0 

1: 000003e9 000003e8 v:1 d:0 
   0001ab9f 00002706 v:1 d:0 

2: 00000401 00000400 v:1 d:0 
   00000401 00000400 v:1 d:0 

3: 0000001f 00000020 v:1 d:0 
   0000001f 00000020 v:1 d:0 

total number of hits: 21
total number of misses: 91
miss rate: 81.2%
total number of dirty blocks: 0
average memory access cycle: 311.7% 
```
