rm -rf *.log
clang -std=c99 -Wall -pedantic *.c /home/dyar/Desktop/ds2/pa4/libruntime.so -o pa4
LD_PRELOAD=/home/dyar/Desktop/ds2/pa4/libruntime.so ./pa4 -p 2 --mutexl
rm -rf pa4