#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>


//currently made for linux, will change once I can dualboot
int main() {
    const char* name = "/chuniio";
    const size_t SIZE = 1024;

    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SIZE);

    void* ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    strcpy((char*)ptr, "Waiting for Unity...");

    printf("Shared memory created. Waiting 30 seconds...\n");
    sleep(10);

    printf("Received from Unity: %s\n", (char*)ptr);

    munmap(ptr, SIZE);
    close(fd);
    shm_unlink(name);
    return 0;
}
