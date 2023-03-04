rm -rf *.log
clang -std=c99 -Wall -pedantic *.c /home/dyar/Desktop/ds2/pa3/libruntime.so -o pa3
LD_PRELOAD=/home/dyar/Desktop/ds2/pa3/libruntime.so ./pa3 -p 2 10 20
rm -rf pa3