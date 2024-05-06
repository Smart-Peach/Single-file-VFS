#include "../includes/AwesomeFileSystem.hpp"
#include "../exceptions/IOException.hpp"

/* TODO: EXTENDS FileSystem.cpp
Create file:
    - Create Inode
    - Add Inode to i-list
    - Add Inode to Superblock (update fields in Superblock --> it will be in Superblock.cpp)
    (I guess we don't need to open bin file here, because in description of "AddInode" in InodeMap it should write to bin file)

*/


//Loads Superblock's fields into second 1024 bytes
void AwesomeFileSystem::load_superblock_into_memory() {
    if (!fs_file.is_open()) {
        throw SuperblockException("Superblock: It's impossible to load, because file is closed");
    }

    fs_file.seekg(1024);
    fs_file << superblock.fs_type;
    fs_file << superblock.sizeof_fs;
    fs_file << superblock.max_sizeof_file;
    fs_file << superblock.sizeof_ilist_bytes;
    fs_file << superblock.number_blocks;
    fs_file << superblock.number_free_blocks;
    fs_file << superblock.number_available_inodes;
    fs_file << superblock.sizeof_block;
    fs_file << superblock.size_of_rootdir;
    fs_file << superblock.free_blocks.to_string();
    fs_file.seekg(0);
}

void AwesomeFileSystem::load_all_into_memory() {
    load_superblock_into_memory();
}

//Goes to the specified address and writes data to the specified boundaries
void AwesomeFileSystem::write_to_file_with_specified_boundaries(int start, int num_of_char, std::string data, int address){
    fs_file.seekg(address);
    fs_file << data.substr(start, num_of_char);
};
//Update info in file's inode after adding one block
void AwesomeFileSystem::update_inode(Inode& inode, int size, int new_address){
    inode.increase_blocks_amount();
    inode.add_size_to_sizeof_file(size);
    inode.update_blocks_storage(new_address);
};

//Reads data from second 1024 bytes and loads it to Superblock
void AwesomeFileSystem::load_superblock_from_memory() {

    if (!fs_file.is_open()) {
        throw SuperblockException("Superblock: It's impossible to load, because file is closed");
    }

    fs_file.seekg(1024);
    fs_file.read((char*)(&superblock.sizeof_fs), sizeof(superblock.sizeof_fs));
    fs_file.read((char*)(&superblock.max_sizeof_file), sizeof(superblock.max_sizeof_file));
    fs_file.read((char*)(&superblock.sizeof_ilist_bytes), sizeof(superblock.sizeof_ilist_bytes));
    // fs_file.read((char*)(&Superblock::number_blocks), sizeof(superblock.number_blocks));
    fs_file.read((char*)(&superblock.number_free_blocks), sizeof(superblock.number_free_blocks));
    fs_file.read((char*)(&superblock.number_available_inodes), sizeof(superblock.number_available_inodes));
    fs_file.read((char*)(&superblock.sizeof_block), sizeof(superblock.sizeof_block));
    fs_file.read((char*)(&superblock.size_of_rootdir), sizeof(superblock.size_of_rootdir));
    fs_file.seekg(2048);
    fs_file.read((char*)(&superblock.free_blocks), sizeof(superblock.free_blocks));
    fs_file.seekg(0);
}

void AwesomeFileSystem::create_file(std::string src_name) {
    if (inode_map.is_file_in_directory(src_name)) {
        int free_block = superblock.get_free_block();
        Inode file_inode = Inode(0, free_block);
        superblock.update_fields_after_inode_addition(file_inode);
        inode_map.add_inode(0, src_name, free_block);  // pass zero - type of src for inode
        load_all_into_memory();
    } else throw IOException("File " + src_name + " already exists!");
};

void AwesomeFileSystem::delete_file(std::string src_name) {
    if (inode_map.is_file_in_directory(src_name)) {
        Inode deleted_inode = inode_map.get_inode(src_name);
        inode_map.delete_inode(src_name);
        superblock.update_fields_after_inode_deletion(deleted_inode);
        load_all_into_memory();
    } else throw IOException("No such file in directory!");
};

void AwesomeFileSystem::write_to_file(std::string src_name, std::string data) { 
    Inode inode = open_file(src_name);
    int sizeof_file = inode.get_sizeof_file();
    int blocks_amount = inode.get_blocks_amount();
    int block_size = superblock.sizeof_block;

    int new_size = sizeof_file + data.size(); //new size for file
    int available_memory = blocks_amount * block_size; //for file on the current time

    int free_memory_in_last_block = available_memory - sizeof_file;
    int shift = block_size - free_memory_in_last_block; // shift for address from start of the last block
    size_t address = inode.get_last_block() + shift;

    if(new_size <= available_memory){ //if we can write all data into last block
        write_to_file_with_specified_boundaries(0, data.size(), data, address);
        fs_file.seekg(0);
        //update info in inode
        inode.add_size_to_sizeof_file(data.size());
        //TODO: update time_t fields in inode
    } else{
        write_to_file_with_specified_boundaries(0, free_memory_in_last_block, data, address); //fill in the last block to the end
        inode.add_size_to_sizeof_file(free_memory_in_last_block);

        int extra_blocks = (new_size - available_memory)/block_size; //the integer part of the number (extra blocks for data)
        if((new_size - available_memory) % block_size > 0){
            extra_blocks ++;  //+ 1 block for data
        }
        if(superblock.check_needed_number_of_free_blocks(extra_blocks)){
            int index = 0;
            while (extra_blocks > 1){
                int new_address = superblock.get_free_block();
                write_to_file_with_specified_boundaries(index, block_size, data, new_address);
                index += block_size;
                
                //update info in inode
                update_inode(inode, block_size, new_address);
                //TODO: update time_t fields in inode

            extra_blocks --;
            }
            int new_address = superblock.get_free_block();
            write_to_file_with_specified_boundaries(index, data.size() - index, data, new_address);
            fs_file.seekg(0);
        } else throw SuperblockException("Not enough memory");
    }
};

//Returns the file's inode
Inode AwesomeFileSystem::open_file(std::string src_name) {
    if (inode_map.is_file_in_directory(src_name)){
        Inode inode = inode_map.get_inode(src_name);
        return inode;
    } else throw IOException("No such file in directory!");
};

void AwesomeFileSystem::read_file(std::string src_name) {
    Inode inode = open_file(src_name);
    std::vector<size_t> storage = inode.get_blocks_storage();
    int num_of_available_char = inode.get_sizeof_file();
    const int block_size = superblock.sizeof_block;

    for(int i = 0; i < storage.size(); i++){
        char c;
        fs_file.seekg(storage[i]); //change location to start of current block
        int count = 0;
        while (fs_file.get(c) && count < block_size && num_of_available_char > 0){
            std::cout << c;
            count++;
            num_of_available_char--;
        }
    }
    fs_file.seekg(0);
}

void AwesomeFileSystem::close_file(std::string src_name) { }

void AwesomeFileSystem::upload_to_file(std::string src_name){ }

// Directory's operations:
void AwesomeFileSystem::create_dir(std::string src_name) {
    Inode dir_inode = open_file(src_name);
 }
void AwesomeFileSystem::delete_dir(std::string src_name) { }
void AwesomeFileSystem::add_file_to_dir(std::string file_name, std::string dir_name) { }
void AwesomeFileSystem::delete_file_in_dir(std::string file_name, std::string dir_name) { }
Inode AwesomeFileSystem::open_dir(std::string src_name) { return Inode(1);}
void AwesomeFileSystem::close_dir(std::string src_name) { }
void AwesomeFileSystem::link_dir(std::string src_name) { }
void AwesomeFileSystem::unlink_dir(std::string src_name) { }
