rm -rf a.out
rm -rf *.log
clang -std=c99 -Wall -pedantic *.c /home/dyar/Desktop/ds2/pa2/libruntime.so -o pa2
LD_PRELOAD=/home/dyar/Desktop/ds2/pa2/libruntime.so ./pa2 -p 3 10 20 30