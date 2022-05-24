#include <iostream>
#include "fs.h"
#include <string>
#include <sstream>
#include <vector>
#include <string.h>
#include <stdlib.h>


int blk_curr_dir = ROOT_BLOCK;

FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
    this->working_block = 0;
    disk_update(0);
 
    cd("/");
    this->working_directory = get_dir_block(this->working_block);
    this->current_pwd_accessrights = "/rwx";
}

FS::~FS()
{
}
/************************ HELP FAT FUNCKTION ************************/
/*
Updating the disk
*/
void 
FS::disk_update(int flag)
{
    //read
    if(flag == 0)
    {
        this->disk.read(this->working_block,(uint8_t*) this->dir_table);       
        this->disk.read(1,(uint8_t*) this->fat);
    }
    //write
    if(flag == 1)
    {
        this->disk.write(this->working_block,(uint8_t*) this->dir_table);
        this->disk.write(1,(uint8_t*) this->fat);
    }
}


std::string 
FS::get_user_input()
{
    std::string lines;
    std::vector<std::string> vector_string;

    while(std::getline(std::cin, lines))
    {
        if(lines.empty()) //Ended with an empty line
        {
            break;
        }
        vector_string.push_back(lines); //Storing user input into the vector
    }
    std::string return_values;
    std::vector<std::string>::iterator x;
    for(x = vector_string.begin(); x !=  vector_string.end(); x++)
    {
        return_values += *x + '\n';
    }

    return return_values;
}

/*
    RETURN THE INDEX OF NEXT FREE  BLOCK 
*/
int
FS::nxt_free_block()
{
    for(size_t i = 2; i < BLOCK_SIZE/2; i++)
    {
        if(this->fat[i] == FAT_FREE)
        {
            return i;
        }
    }
    return -1;
}

/*
    read the text of the first block of Fat blocks
*/

std::string
FS::reading_disk(uint16_t first_block)
{
    uint8_t temp_disk_read[BLOCK_SIZE];
    std::string read_data;
    do
    {
        this->disk.read(first_block, temp_disk_read);
        for (size_t i=0; i<BLOCK_SIZE; i++)
        {
            if((char)temp_disk_read[i]==(char)4)
            {
                break;
            }
            read_data += (char)temp_disk_read[i];
        }
        first_block=fat[first_block];
    } while (first_block != Max_value);
    
    return read_data += (char)4;
    
}


/*
Add new directory table
*/
int
FS::insert_dir_entry(dir_entry d)
{
    for(size_t i=0; i<BLOCK_SIZE/64; i++)
    {
        if(this->dir_table[i].type==2) // free/unused dir = 2
         {
            this->dir_table[i]=d;
            //Succesfull = 0
            return 0;
        }
    }
    //Unsucceull = -1
    return -1;
}

/*
Getting directory entry from the directory table by using its filename
*/
dir_entry* 
FS::get_dir_name(std::string name)
{
    for(size_t i = 0; i < BLOCK_SIZE/64; i++)
    {
        if(this->dir_table[i].type != 2 && this->dir_table[i].file_name == name)
        {
            //Succefull = pointer to directory
            return &this->dir_table[i];
        }
    }
    // Unsuccefull = NULL
    return nullptr;
}

/*
Get directory entry from the directory table by using its first_block
*/
dir_entry*
FS::get_dir_block(int fat_block)
{
    for(size_t i=0; i< BLOCK_SIZE / 64; i++)
    {
        if(this->dir_table[i].first_blk == fat_block)
        {
            return &this->dir_table[i];
        }
    }
    return nullptr;
}


// Printing the access rights
std::string
FS::print_accesrights(int arguments)
{
    std::string ret_str = "";
    int result = READ & arguments;
    if(result > 0)
    {
        ret_str.push_back('r');
    }
    else
    {
        ret_str.push_back('-');
    }
    result = WRITE & arguments;
    if(result > 0)
    {
        ret_str.push_back('w');
    }
    else
    {
        ret_str.push_back('-');
    }
    result = EXECUTE & arguments;
    if(result > 0)
    {
        ret_str.push_back('x');
    }
    else
    {
        ret_str.push_back('-');
    }
    return ret_str;
}

//Getting the access rigt from the current directory 
uint8_t
FS::get_current_access_right()
{
    std::string substring;
    uint8_t access_right = 0;
    try
    {
        substring = this->current_pwd_accessrights.substr(this->current_pwd_accessrights.find_last_of("/"));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    substring.erase(0,1);

    for(char &which_access_right : substring)
    {
        switch(which_access_right)
        {
        case 'r':
            access_right += READ;
            break;
        case 'w':
            access_right += WRITE;
            break;

        case 'x':
            access_right += EXECUTE;
            break;
        default:
            break;
     
        }   
    }
    
    return access_right;
}

std::string
FS::check_path(std::string filepath, bool avoid_rights = true)
{
    if(filepath.length() > filepath.find('/')) 
    {
        size_t check = filepath.find_last_of("/\\");
        hp_cd(filepath.substr(0, check), avoid_rights);
        filepath = filepath.substr(check+1); //change the file path to spacific dir
    }
    return filepath;
}

std::string
FS::erase_check_path(std::string filepath, bool avoid_rights = true)
{
    cd("/");
    filepath.erase(0,1);
    std::cout << filepath << std::endl;
    return check_path(filepath, avoid_rights);
}

/*
    Set all Fat blocks to zero(free) by using Recursive method
*/
int
FS::clear_file(int first_block)
{
    if(this->fat[first_block] != FAT_EOF)
    {
        clear_file(this->fat[first_block]);
    }
    this->fat[first_block]=FAT_FREE;

    return 0;
}

/************************ THE COMANDS HELP***************************/
//Help function to Create command creating file with accesright rwx = 7 as default

int 
FS::hp_create(std::string filepath, std::string input_content, uint8_t acces_right = 7, bool avoid_rights = false, bool flag = 0)
{
    disk_update(0);
    std::string cur_path = this->current_pwd;
    std::string file_name;

    int ongoing_block = nxt_free_block();
    // look and erase the / if found
    if(filepath[0] == '/')
    {
        file_name = erase_check_path(filepath);    
    }
    else
    {
        file_name = check_path(filepath);
    }    
    if(flag == 1 || get_dir_name(file_name) == nullptr)
    {
        //Checking if the user have premission to write to this file
        if(this->current_pwd != "" && !(get_current_access_right() & WRITE))
        {
            std::cout << "You dont have the premission to write to this file path: " << this->current_pwd << std::endl;
            cd(cur_path);
            this->current_pwd = cur_path;
        }
        if(input_content.back() != (char)4)
        {
            input_content.append(std::to_string((char)4));
        }
        dir_entry created_file;
        strcpy(created_file.file_name, file_name.c_str());
        created_file.size = input_content.size();
        created_file.first_blk = ongoing_block;
        created_file.type = 0;
        created_file.access_rights = acces_right; //rwx as Deafult
    
        while(true)
        {
            this->fat[ongoing_block] = FAT_BLOCK; //setting current block index  as blocked so nxt_free_block cant return it 
            if (input_content.size() >= BLOCK_SIZE)
            {
                this->disk.write(ongoing_block, (uint8_t*)input_content.substr(0, BLOCK_SIZE).c_str());
                input_content.erase(0, BLOCK_SIZE);
            }
            else
            {
                this->disk.write(ongoing_block, (uint8_t*)input_content.c_str());
                this->fat[ongoing_block] = FAT_EOF; // the last block used is in the end of the file 
                break;
            }
            int block_index = nxt_free_block();
            this->fat[ongoing_block] = block_index;
            ongoing_block = block_index;
        }
        try
        {
            insert_dir_entry(created_file);
        }
        catch(int error)
        {
            std::cout << "Expection has occured 'NO DIR' " << error << std::endl;
        }
        disk_update(1);
    }
    else
    {
        std::cout << "error: Invalid file name or directory" << std::endl;
    }
    cd(cur_path);
    this->current_pwd = cur_path; 

    return 0;
}

/*
    help cd function for Cd command
*/
int
FS::hp_cd(std::string dirpath, bool avoid_rights)
{
    if(dirpath.empty())
    {
        dirpath = "/";
    }
    if(dirpath[0] == '/')
    {
        current_pwd = "";
    }
    else
    {
        if(this->current_pwd.back() == '/')
        {
            this->current_pwd.pop_back();
        }
    }
    if(dirpath == "..")
    {
        this->working_block = this->dir_table[0].first_blk;
        this->current_pwd = this->current_pwd.substr(0, this->current_pwd.find_last_of("/"));
        this->current_pwd_accessrights = this->current_pwd_accessrights.substr(0, this->current_pwd_accessrights.find_last_of("/"));
        disk_update(0);
    }
    else if(dirpath == "/")
    {
        this->working_block = 0;
        disk_update(0);
    }
    else
    {
        std::string current_dir;
        int end_of_path = 0;
        while(current_dir != dirpath)
        {
            end_of_path = dirpath.find("/"); 
            if(end_of_path == (int)std::string::npos)
            {
                current_dir = dirpath;
            }
            else
            {
                current_dir = dirpath.substr(0, end_of_path);
                dirpath.erase(0, end_of_path+1);
            }

            //Changing of directory
            if(current_dir == "")
            {
                this->working_block = 0;
            }
            else
            {
                dir_entry *temporary_dir = get_dir_name(current_dir);
                if(temporary_dir->type == 0 || temporary_dir->type == 2 || temporary_dir == nullptr) //  If it not a directory, we break
                {
                    break;
                }
                if(!(temporary_dir->access_rights & EXECUTE) && avoid_rights == false)
                {
                    std::cout << "You dont have the rights to access the following directory: " << temporary_dir->file_name << std::endl;
                    break;
                }
                this->current_pwd += "/" + current_dir;
                this->current_pwd_accessrights += "/" + print_accesrights(temporary_dir->access_rights);
                this->working_block = temporary_dir->first_blk;
                this->working_directory = temporary_dir;
            }
            disk_update(0);
        }
    }
    return 0;
}

/*
    help function to remove command
*/
int
FS::hp_rm(std::string filepath, bool avoid_rights)
{
    std::string cur_path = this->current_pwd;
    std::string file_name;
    
    if(filepath[0] == '/')
    {
        file_name = erase_check_path(filepath);
    }
    else if(filepath.find('/') != std::string::npos)
    {
        file_name = check_path(filepath);
    }   
    else
    {
        file_name = filepath;
    }
    disk_update(0);

    if(!(get_current_access_right() & WRITE) && avoid_rights == false)
    {
        std::cout << "You dont have the right to delete the file from the following path: " << this->current_pwd << std::endl;
        cd(cur_path);
        this->current_pwd = cur_path;
        return -1;
    }

    dir_entry* current_directory = get_dir_name(file_name);
    if(current_directory == 0) // if it zero means there is no file or dir to delete and free the block
    {
        std::cout << "The file: " << filepath << " does not exists " << std::endl;
        return -1; 
    }
    
    if(current_directory->access_rights & WRITE)
    {
        current_directory->size= 0;
        if(current_directory->type == 1)
        {
            std::string fileName;
            if(filepath[0] == '/')
            {
                fileName = erase_check_path(filepath);
            } 
            else
            {
                fileName = filepath;
            }
            cd(current_directory->file_name);
            if(get_dir_name(fileName) != nullptr)
            {
                current_directory->size++;
                disk_update(1);
            }

            if(current_directory->size > 1)
            {
                std::cout << "You can not delete a directory that is not empty" << std::endl;
                cd(cur_path);
                this->current_pwd = cur_path;
                return -1; 
            }
            else 
            {
                cd(cur_path);
                current_directory->type = 2;
                this->fat[current_directory->first_blk] = FAT_EOF;
                disk_update(1);
            }
        }   
        
        if(this->fat[current_directory->first_blk] != FAT_EOF && current_directory->type == 0)
        {
            clear_file(this->fat[current_directory->first_blk]);
        }
        else
        {
            current_directory->type = 2;
            this->fat[current_directory->first_blk] = FAT_EOF;
            disk_update(1);
        }

    }
    else
    {
        std::cout << "error: Change access rights, this command is not allowed" << std::endl;
    }
    
    cd(cur_path);
    this->current_pwd = cur_path;

    return 0;
}   



/*************************ALL THE COMMANDS*************************/
int
FS::format()
{
    this->fat[0] = ROOT_BLOCK; 
    this->fat[1] = FAT_BLOCK;
    // set the block to zeros
    for (size_t i = 2; i < BLOCK_SIZE/2; i++)
    {
        this->fat[i] = FAT_FREE;
    }
    this->disk.write(1, (uint8_t*) this->fat);
    
    dir_entry formating_directory;
    formating_directory.type = 2;   // 2 = free directory
    for(size_t i = 0; i < BLOCK_SIZE/64; i++) //Filling the dir table with free blocks
    {
        this->dir_table[i] = formating_directory;
    }
    for (size_t i = 0; i < this->disk.get_no_blocks(); i++) //Setting all block as free
    {
        this->disk.write(i, (uint8_t*)this->dir_table);
    }
    
    dir_entry root_directory;
    root_directory.first_blk = 0;
    root_directory.type = 1;
    root_directory.access_rights = 7;    

    this->dir_table[0] = root_directory; //Setting the root directory in the first place in the dir table
    this->disk.write(0, (uint8_t*)this->dir_table);
    cd("/");
    return 0;
} 

int
FS::create(std::string filepath)
{
    if(filepath.length() > 55) //Checking if string path of the file longer than 55 characters
    {
        std::cout << "Error: filename is too long, must be less than 55 characters!" << std::endl; 
        return -1;
    }
    
    size_t found =  filepath.find_last_of('/');
    if(found == SIZE_MAX)
    {
        found = 0;
    }
    if(get_dir_name(filepath.substr(found, filepath.length())) == nullptr)
    {
        std::string user_input = get_user_input(); 
        if(user_input.length() != 0)
        {
            user_input.pop_back(); //Deleting last empty line if user input is not empty
        }
        user_input+= (char)4;
        hp_create(filepath, user_input);
    }
    else    
    {
        std::cout << "error: Invalid file name or dir" << std::endl;
    }
    return 0;
}

int
FS::cat(std::string filepath)
{
    std::string cur_path = this->current_pwd;

    //If the filepath is more than the filename then the filename wil be created in the specific directory
    if(filepath.length() > filepath.find('/')) 
    {
        size_t found = filepath.find_last_of("/\\");
        cd(filepath.substr(0, found));
        filepath = filepath.substr(found+1);
    }
    
    dir_entry* current_directory = get_dir_name(filepath);
    if(current_directory == nullptr)
    {
        std::cout << "error: File does not exists" << std::endl;
    }
    else
    {
        disk_update(0);
        if(current_directory->type == 0 && (current_directory->access_rights & READ)) //Are you allowed to read the file and is it a directory?
        {
            std::string output = reading_disk(current_directory->first_blk);
            std::cout << output.substr(0, output.length()-1) << std::endl;
        }
        else if(current_directory->type == 1)
        {
            std::cout << "error: Can not read content from a directory" << std::endl;
        }
        else
        {
            std::cout << "error: Check premission" << std::endl;
        }    
    }
    cd(cur_path);
    this->current_pwd = cur_path;

    return 0;
}

int
FS::ls()
{
    if(get_current_access_right() & READ)
    {
        std::cout << "name\t\ttype\t\taccessrights\tsize" << std::endl; //Columns
        std::cout << "..\t\tdir\t\trwx\t\t-" << std::endl; //Parent directory

        for (size_t i=1; i<BLOCK_SIZE/64; i++)
        {
            if(this->dir_table[i].type !=2 && this->dir_table[i].first_blk <= this->disk.get_no_blocks())
            {
                if(this->dir_table[i].type == 0 && strcmp(this->dir_table[i].file_name,"") !=0)
                {
                    std::cout<< this->dir_table[i].file_name <<"\t\tfile\t\t" << print_accesrights(this->dir_table[i].access_rights) <<"\t\t" <<this->dir_table[i].size << std::endl;
                }
                else if(this->dir_table[i].type ==1)
                {
                    std::cout<< this->dir_table[i].file_name <<"\t\tdir\t\t" << print_accesrights(this->dir_table[i].access_rights) << "\t\t-" << std::endl;
                }
            }
        }
    }
    else
    {
        std::cout << "error: Your are not allowed to read content from this current directory" << std::endl;
    }

    return 0;
}

int
FS::cp(std::string sourcepath, std::string destpath)
{
    disk_update(0);
    std::string cur_path = this->current_pwd;
    
    if(sourcepath == destpath)
    {
        std::cout << "error: The copy can not have the same name as the source file when both files is in the same dir" << std::endl;
        return -1;
    }

    else if(get_dir_name(destpath) != nullptr && get_dir_name(destpath)->type == 0)
    {
        std::cout << "error: There is already a file with that name" << std::endl;
        return -1;
    }
    if(sourcepath[0] == '/')
    {
        sourcepath = erase_check_path(sourcepath, false);
    }
    else
    {
        sourcepath = check_path(sourcepath, false);
    }

    dir_entry* source_path = get_dir_name(sourcepath);
    
    if(source_path == nullptr)
    {
        std::cout << "error: The source file does not exist" << std::endl;
        return -1;
    }
    std::string source_content = reading_disk(source_path->first_blk);
    if(source_path->access_rights & READ) //Are you allowed to read the content from the source file
    {
        dir_entry *destination_path = get_dir_name(destpath);
        
        if(destpath[0] == '/' || destpath[0] == '.') //when the user wants to copy a file to a dir
        {
            int dest_dir_type;
            std::string dest_file;
            dir_entry* dest_dir;
            std::string sourcefile;
            if(destpath[0] == '/')
            {
                dest_file = erase_check_path(destpath);
            }
            else
            {
                dest_file = check_path(destpath);
            }
            if(dest_file == "")
            {
                dest_dir = get_dir_name(".");
                dest_dir_type = 1;
            }
            else
            {
                dest_dir = get_dir_name(dest_file);
                if(dest_dir != nullptr)
                {
                    dest_dir_type = dest_dir->type;
                } 
            }
            cd(cur_path);

            if(sourcepath[0] == '/')
            {
                sourcefile = erase_check_path(sourcepath);
            }
            else
            {
                sourcefile = check_path(sourcepath);
            }
            if(dest_dir_type == 1)
            {
                destpath += "/" + sourcefile;  
                hp_create(destpath, source_content, source_path->access_rights, false, 1);
                disk_update(1);  
            }   
        }
        else // when the user only wants copy a file
        {
            cd(cur_path);
            hp_create(destpath, source_content, source_path->access_rights, false, 0);
            disk_update(1);
        }
    }
    else
    {
        std::cout << "error: You are not allowed to read the content, check premission" << std::endl;
    }
    
    return 0;
}

int
FS::mv(std::string sourcepath, std::string destpath)
{
    disk_update(0);
    std::string cur_path= this->current_pwd;
    std::string source_file;
    std::string dest_file;
    dir_entry* dest_dir;
   
    int dest_dir_type = 0;
    if(get_dir_name(destpath) != nullptr && get_dir_name(destpath)->type == 0)
    {
        std::cout << "error: There is already a file with that name" << std::endl;
        return -1;
    }

    if(destpath[0] == '/')
    {
        dest_file = erase_check_path(destpath);
    }
    else
    {
        dest_file = check_path(destpath);
    }
    if(dest_file == "")
    {
        dest_dir = get_dir_name(".");
        dest_dir_type = 1;
    }
    else
    {
        dest_dir = get_dir_name(dest_file);
        if(dest_dir != nullptr)
        {
            dest_dir_type = dest_dir->type;
        } 
    }

    cd(cur_path);
    if(sourcepath[0] == '/')
    {
        source_file = erase_check_path(sourcepath);
    }
    else
    {
        source_file = check_path(sourcepath);
    }
    dir_entry* source_dir = get_dir_name(source_file);
    if(dest_dir_type == 1) //Moving the file into a directory++;
    {
        std::string content = reading_disk(source_dir->first_blk);
        destpath += "/" + source_file;
        if(this->fat[source_dir->first_blk] != FAT_EOF && source_dir->type == 0)
        {
            clear_file(this->fat[source_dir->first_blk]);
        }
        source_dir->type = 2;
        this->fat[source_dir->first_blk] = FAT_EOF;
        disk_update(1);
        hp_create(destpath, content, source_dir->access_rights);
        cd(cur_path);
        disk_update(1);
    } 

    if(source_dir == nullptr)
    {
        std::cout << " The file: " << source_file.c_str() << " does not exists " << std::endl;
        return -1;
    }

    if((source_dir->access_rights & WRITE) > 0) //Is the user allowed to change with the source file
    {
         if(destpath.length() > 55)
        {
            std::cout << "The renamed file is too long" << std::endl;
            return 1;
        }
        strcpy(source_dir->file_name, destpath.c_str()); //Rename to the destpath
        
        disk_update(1); //update the disk to 1 (writing)
    }
    else
    {
        std::cout << "error: You are not allowed this command, check premission" << std::endl;
    }
    cd(cur_path);
    this->current_pwd = cur_path;

    return 0;
}

int
FS::rm(std::string filepath)
{
    hp_rm(filepath, false);
    return 0;   
}

int
FS::append(std::string filepath1, std::string filepath2)
{
    disk_update(0);
    std::string cur_path = this->current_pwd;
    std::string cur_path2 = filepath2;
    std::string file_name1;
    std::string file_name2;

    if(filepath1[0] == '/')
    {
        file_name1 = erase_check_path(filepath1);
    }
    else
    {
        file_name1 = check_path(filepath1);
    }    

    dir_entry *dir1 = get_dir_name(file_name1);
    // read from first file
    if(dir1 == nullptr)
    {
        std::cout << "error: File " << filepath1.c_str() << " does not exists " << std::endl;
        cd(cur_path);
        this->current_pwd = cur_path;
        return -1;
    }
    int check_access_dir1 = dir1->access_rights & READ;
    int file1_first_block = dir1->first_blk;

    cd(cur_path);

    if(filepath2[0] == '/')
    {
        file_name2 = erase_check_path(filepath2);
    }
    else
    {
        file_name2 = check_path(filepath2);
    }  
    //write to the second file  
    dir_entry *dir2 = get_dir_name(file_name2);
    if(dir2 == nullptr)
    {
        std::cout << "error: File " << filepath2.c_str() << " does not exists " << std::endl;
        cd(cur_path);
        this->current_pwd = cur_path;
        return -1;
    }
    int check_access_dir2 = dir2->access_rights & WRITE;
    int file2_first_block = dir2->first_blk;

    if(check_access_dir1 > 0 && check_access_dir2 > 0)
    {
        //FEL
        std::string content = reading_disk(file2_first_block); //Reading first data from file2
        content.pop_back(); //Delete EOF
        content += "\n" + reading_disk(file1_first_block); //plus data from file 1
        cd(cur_path);
        hp_rm(cur_path2, true);
        hp_create(cur_path2, content, dir2->access_rights, false, 1);
        disk_update(1);
    
    }
    else
    {
        std::cout << "error: This command is not allowed, check access rights" << std::endl;
    }
     
    cd(cur_path);
    this->current_pwd = cur_path;

    return 0;
}

int
FS::mkdir(std::string dirpath)
{
    disk_update(0);
    std::string cur_path = this->current_pwd;
    std::string directory;
    
    if(dirpath[0] == '/')
    {
        cd("/");
        dirpath.erase(0,1);
    }
    for(;;)
    {
        if(dirpath.length() == 0)
        {
            break;
        }
        size_t found = dirpath.find("/");
        if(found == SIZE_MAX)
        {
            found = 0;
            directory = dirpath;
        }
        else
        {
            directory = dirpath.substr(0, found);
        }
        if(get_dir_name(directory) != nullptr)
        {
            if((get_dir_name(dirpath) != nullptr && get_dir_name(dirpath)->type == 1))
            {
                std::cout << "error: There is already a directory with that name" << std::endl;
            }
            if((get_dir_name(dirpath) != nullptr && get_dir_name(dirpath)->type == 0))
            {
                std::cout << "error: There is already a file with that name" << std::endl;
            }

            cd(directory);
            dirpath.erase(0,found+1);
            continue;
        }
        
        int block_index = nxt_free_block();
        //creating sub-directory
        dir_entry sub_dir;
        strcpy(sub_dir.file_name, directory.c_str());
        sub_dir.first_blk = block_index;
        sub_dir.type = 1;
        sub_dir.access_rights = 7;
        insert_dir_entry(sub_dir);
        disk_update(1);

        //Creating parent entry for every new sub-directory (..)
        dir_entry mkdir_table[BLOCK_SIZE/64];
        dir_entry mkdir_root;
        strcpy(mkdir_root.file_name, "..");
        mkdir_root.first_blk = this->working_block; //Pointing to the block where the parent directory is stored
        mkdir_root.type = 1;
        mkdir_root.access_rights = 7; 
        mkdir_table[0] = mkdir_root;
      
        //Filling thw new sub directory with empty entries (ready to use)
        dir_entry format_directory;
        format_directory.type = 2; 
        for(size_t i = 1; i < BLOCK_SIZE/64; i++)
        {
            mkdir_table[i] = format_directory;
        }
        
        this->disk.write(block_index, (uint8_t*) mkdir_table);
        this->fat[block_index] = FAT_BLOCK;
        disk_update(1);

        cd(directory);

        if(found > 0)
        {
            dirpath.erase(0, found+1);
        }
        else
        {
            dirpath = "";
        }
        if(found == std::string::npos)
        {
            break;
        }
    }
    if(cur_path == "")
    {
        cd("/");
    }
    else
    {
        cd(cur_path);
    }
    
    return 0;
}

int
FS::cd(std::string dirpath)
{
    hp_cd(dirpath, false);
    return 0;
}

int
FS::pwd()
{
    if(this->current_pwd == "")
    {
        std::cout << "/";
    }
    // Print the path after the /
    std::cout << this->current_pwd << std::endl;
    return 0;
}

int
FS::chmod(std::string accessrights, std::string filepath)
{   
    std::string cur_path = this->current_pwd;
    
    if(get_dir_name(filepath) == nullptr)
    {
        std::cout << "error: Selection does not exist" << std::endl;
        return -1;
    }
    if(std::stoi(accessrights) == 0 || std::stoi(accessrights) > 7) 
    {
        std::cout << "error: You must select access rights between 1-7" << std::endl;
    }

    if(filepath.length() > filepath.find('/'))
    {
        size_t found = filepath.find_last_of("/\\");
        cd(filepath.substr(0, found));
        filepath = filepath.substr(found+1);
    }
    disk_update(0);

    dir_entry* current_file = get_dir_name(filepath); 
    current_file->access_rights = std::stoi(accessrights);
    disk_update(1);

    cd(cur_path);
    this->current_pwd = cur_path;

    return 0;
}
