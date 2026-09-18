#include <stdio.h>

int main() {

    int page_table[] = {3, 5, 1, 7};
    int page_size = 100;

    int virtual_address = 250;

    int page = virtual_address / page_size;
    int offset = virtual_address % page_size;

    int frame = page_table[page];

    int physical_address = frame * page_size + offset;

    printf("Virtual Address   = %d\n", virtual_address);
    printf("Page Number       = %d\n", page);
    printf("Offset            = %d\n", offset);
    printf("Frame Number      = %d\n", frame);
    printf("Physical Address  = %d\n", physical_address);

    return 0;
}
