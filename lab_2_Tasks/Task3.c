#include <stdio.h>
#include <stdlib.h>

//Using a circular array that overwrite the oldest page with a new page
void FIFO(int _pages, int page_size, FILE *file_name)
{
    int memory_frames[_pages*page_size];
    int page_faults = 0; 
    int file_reference = 0;
    int virt_adrr = 0;
    int page_exists;
    
    char *linebuff = NULL;
    size_t n = 0;
    ssize_t read_lines;

    while((read_lines = getline(&linebuff,&n, file_name) != -1))
    {
        int memory_add = atoi(linebuff);
        page_exists = 0;

        //Checking if the memory page alreade exists in the page table
        for(size_t i = 0; i < _pages * page_size; i++)
        {
            if(memory_frames[i] == memory_add)
            {
                page_exists = 1;
                break;
            }
        }
        if(page_exists == 0)
        {
            page_faults++; //Increasing page fault 
            memory_add = (memory_add / page_size) * page_size; // Creating first memory page in the page
            //Adding all the memory references in the page to the memory
            for(size_t i = page_size*(virt_adrr % _pages); i < page_size*((virt_adrr%_pages) +1) ; i++)
            {
                memory_frames[i] = memory_add;
                memory_add++;  
            }
            virt_adrr++;
        }
        file_reference++;
    }
    printf("Read %d memory reference => %d pagefaults\n", file_reference, page_faults);
    
}

int main(int argc, char **argv)
{
    printf("No physical pages = %s, page size = %s\n", argv[1], argv[2]);
    FILE *file_ptr;

    printf("Reading from memory from %s\n", argv[3]);

    if(file_ptr = fopen(argv[3], "r")) //File exists
    {
        FIFO(atoi(argv[1]), atoi(argv[2]), file_ptr);
    }
    else //File dosent exists
    {
        printf("Error with opening the file\n");
        exit(1);
    }
}