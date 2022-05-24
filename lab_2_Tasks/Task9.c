#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

struct Array
{
    int *array;
    int size;
    int used_page;

};

//Initialzing the array
void init_array(struct Array *a, int initial_size)
{
    a->array = malloc(initial_size * sizeof(int));
    a->size = initial_size;
    a->used_page = 0;
}

//Checking if the page exists in the page table
int exist_page(int page_table[], int size, int index)
{
    for(int i = 0; i < size; i++)
    {
        if(page_table[i] == index)
        {
            return 1;
        }
    }
    return 0;
}  

//Inserting into the array
void insert_in_array(struct Array *a, int element)
{
    if(a->used_page == a->size)
    {
        a->size = a->size * 2;
        a->array = realloc(a->array, a->size * sizeof(int));
    }
    a->array[a->used_page++] = element;
    
}

//Getting the next position when the page is needed
int get_next_pos(struct Array *a, int start_position, int index)
{
    for(int i = start_position; i < a->used_page; i++)
    {
        if(a->array[i] == index)
        {
            return i - start_position;

        } 
    }
    return INT_MAX;
}

void OPT(int no_pages, int page_size, FILE *file_name)
{
    struct Array a;
    init_array(&a, 100);

    int page_table[no_pages];
    int page_faults = 0;
    int file_reference = 0;
    
    int next_pos = 0;
    int farthest_pos;
    int farthest_index;
    
    char *linebuff = NULL;
    size_t n = 0;
    ssize_t read_lines;

    while((read_lines = getline(&linebuff,&n, file_name) != -1))
    {
        int page = (atoi(linebuff) / page_size) * page_size; //What page it belongs to
        insert_in_array(&a, page);
    }

    //Here start the Optimal page replacement
    for (int i = 0; i < a.used_page; i++)
    {
        farthest_pos = 0;
        if(exist_page(page_table, no_pages, a.array[i]) == 0)
        {
            page_faults++;
            if(file_reference < no_pages)
            {
                page_table[file_reference] = a.array[i];
                file_reference++;
                continue;
            }
            for (size_t x = 0; x < no_pages; x++)
            {
                int result = get_next_pos(&a, file_reference, page_table[x]);
                if(result > farthest_pos) //Getting the page that is furthest away
                {
                    farthest_pos = result;
                    farthest_index = x;
                }

            }  
            page_table[farthest_index] = a.array[i]; //Replacing the page thiat is furthest away
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
        OPT(atoi(argv[1]), atoi(argv[2]), file_ptr);
        return 0;
    }
    else //File dosent exists
    {
        printf("Error with opening the file");
        exit(1);
    }
}