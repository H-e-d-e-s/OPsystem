#include <stdio.h>
#include <stdlib.h>

void LRU(int num_pages, int page_size, FILE *file_name)
{
    int page_table[num_pages];
    int page_faults = 0;
    int file_reference = 0;
    int page_swapper;
    int page_exists;

    char *linebuff = NULL;
    size_t n = 0;
    ssize_t read_lines;

    // The least recenlty used page is in the first position of the page table
    while((read_lines = getline(&linebuff,&n, file_name) != -1))
    {
        int page = (atoi(linebuff) / page_size) * page_size; //What page it belongs to
        page_exists = 0;

        for(size_t i = 0; i < num_pages; i++)
        {
            if(page_table[i] == page)
            {
                page_exists = 1; // page already exists in page table
                for(size_t x = i; x < num_pages - 1; x++) //Moving the recently used page to the end of the array 
                {
                    page_swapper = page_table[x+1];
                    page_table[x+1]  = page_table[x];
                    page_table[x] = page_swapper;
                }
                break;
            }
        }
        if(page_exists == 0)
        {
            page_faults++;
            page_table[0] = page; //Replacing the LRU page
            for (size_t i = 0; i < num_pages - 1; i++) //Moving the recently used page to the end of the array 
            {
                page_swapper = page_table[i+1];
                page_table[i+1]  = page_table[i];
                page_table[i] = page_swapper;
            }
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
        LRU(atoi(argv[1]), atoi(argv[2]), file_ptr);
    }
    else //File dosent exists
    {
        printf("Error with opening the file");
        exit(1);
    }
}