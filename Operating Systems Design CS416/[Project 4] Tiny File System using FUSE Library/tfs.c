/*
 *  Copyright (C) 2021 CS416 Rutgers CS
 *	Tiny File System
 *	File:	tfs.c
 *	iLab Used: kill.cs.rutgers.edu
 *	Single person group: Michael Zhang
 *	NetID: mbz27
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <libgen.h>
#include <limits.h>

#include <math.h>

#include "block.h"
#include "tfs.h"

char diskfile_path[PATH_MAX];

// Declare your in-memory data structures here
struct superblock* superBlock = NULL;
bitmap_t inodeBitmap = NULL;
bitmap_t dataBitmap = NULL;

pthread_mutex_t mutex;


/* 
 * Get available inode number from bitmap and sets valid bit
 */
int get_avail_ino() {

	// Step 1: Read inode bitmap from disk
	bio_read(superBlock->i_bitmap_blk, inodeBitmap);
	// Step 2: Traverse inode bitmap to find an available slot
	for(int i = 0; i < MAX_INUM; i++){
		if(get_bitmap(inodeBitmap, i) == 0){
			// Step 3: Update inode bitmap and write to disk 
			set_bitmap(inodeBitmap, i);
			bio_write(superBlock->i_bitmap_blk, inodeBitmap);
			return i;
		}
	}
	return -1;
}

/* 
 * Get available data block number from bitmap and sets valid bit
 */
int get_avail_blkno() {

	// Step 1: Read data block bitmap from disk
	bio_read(superBlock->d_bitmap_blk, dataBitmap);
	// Step 2: Traverse data block bitmap to find an available slot
	for(int i = 0; i < MAX_DNUM; i++){
		if(get_bitmap(dataBitmap, i) == 0){
			// Step 3: Update data block bitmap and write to disk 
			set_bitmap(dataBitmap, i);
			bio_write(superBlock->d_bitmap_blk, dataBitmap);
			return superBlock->d_start_blk + i;
		}
	}
	return -1;
}

/* 
 * inode operations
 */

/*
 * Reads in the inode no matter if it is valid or not
*/
int readi(uint16_t ino, struct inode *inode) {

  	// Step 1: Get the inode's on-disk block number
	int numInodes = (int)floor( (double) BLOCK_SIZE / (double) sizeof(struct inode));
	int iBlock = superBlock->i_start_blk + (int)floor( (double) ino / (double) numInodes);
	// Step 2: Get offset of the inode in the inode on-disk block
	int iOffset = ino % numInodes;
	// Step 3: Read the block from disk and then copy into inode structure
	struct inode* block = (struct inode*)malloc(BLOCK_SIZE);
	bio_read(iBlock, block);
	*inode = *(block + iOffset);
	free(block);

	return 1;
}

void debug(int currInode){
	struct inode* curr = (struct inode*)malloc(sizeof(struct inode));
	readi(currInode, curr);
	if(curr->type != (1)){
		printf("--inode: %d--\n", curr->ino);
		for(int i = 0; i < ((int)ceil((double)curr->size / (double)BLOCK_SIZE)); i++){
			int blockNum = curr->direct_ptr[i];
			if(blockNum == -1 || i > 15){
				// This is probably impossible (mostly for sanity check)
				break;
			}
			struct dirent* currDir = (struct dirent*)calloc(1, BLOCK_SIZE);
			bio_read(blockNum, currDir);
			for(int j = 0; j < ((int)floor((double)BLOCK_SIZE / (double)sizeof(struct dirent))); j++){
				if((currDir+j)->valid == 1){
					printf("%s\n", (currDir+j)->name);
				}
			}
			printf("\n---------------\n");
			for(int j = 0; j < ((int)floor((double)BLOCK_SIZE / (double)sizeof(struct dirent))); j++){
				if((currDir+j)->valid == 1){
					if(strcmp(".", (currDir+j)->name) == 0 || strcmp("..", (currDir+j)->name) == 0){
					}else{
						//debug((currDir+j)->ino);
					}
				}
			}

			free(currDir);
		}
	}
	printf("\n--END--\n");
	free(curr);
	return;
}

/*
 * Writes to inode no matter if it is valid or not
*/
int writei(uint16_t ino, struct inode *inode) {

	// Step 1: Get the block number where this inode resides on disk
	double numInodes = floor( (double) BLOCK_SIZE / (double) sizeof(struct inode));
	int iBlock = superBlock->i_start_blk + (int)floor( (double) ino / (double) numInodes);
	// Step 2: Get the offset in the block where this inode resides on disk
	int iOffset = ino % (int)numInodes;
	// Step 3: Write inode to disk 
	struct inode* block = (struct inode*)calloc(1, BLOCK_SIZE);
	bio_read(iBlock, block);
	*(block + iOffset) = *inode;
	bio_write(iBlock, block);
	free(block);
	return 1;
}


/* 
 * directory operations
 */

/*
 * Finds the the directory given an inode
*/
int dir_find(uint16_t ino, const char *fname, size_t name_len, struct dirent *dirent) {

	// Step 1: Call readi() to get the inode using ino (inode number of current directory)
	struct inode* iDir = (struct inode*)calloc(1, sizeof(struct inode));
	readi(ino, iDir);

	// I might not need this VVVVV
	if(iDir->valid == 0 /*|| iDir->type == (1)*/){ // Makes sure it only has direct ptrs to dirents
		free(iDir);
		return -1;
	}
	// Step 2: Get data block of current directory from inode
	for(int i = 0; i < ((int)ceil((double)iDir->size / (double)BLOCK_SIZE)) /* Number of blocks */; i++){
		int blockNum = iDir->direct_ptr[i];
		if(blockNum == -1 || i > 15){
			// This is probably impossible (mostly for sanity check)
			free(iDir);
			return -1;
		}
		struct dirent* currBlock = (struct dirent*)calloc(1, BLOCK_SIZE);
		bio_read(blockNum, currBlock);
		for(int j = 0; j < ((int)floor((double)BLOCK_SIZE / (double)sizeof(struct dirent))) /* Number of dirents in a block */; j++){
			if((currBlock+j)->valid == 1){
				if(strcmp(fname, (currBlock+j)->name) == 0){
					// Step 3: Read directory's data block and check each directory entry.
					//If the name matches, then copy directory entry to dirent structure
					*(dirent) = *(currBlock+j);
					free(currBlock);
					free(iDir);
					return 1;
				}
			}
		}
		free(currBlock);
	}
	free(iDir);
	return -1;
}

int dir_add(struct inode dir_inode, uint16_t f_ino, const char *fname, size_t name_len) {

	if(dir_inode.type == (1)){ // Makes sure the inode only contains ptrs to dirents (Probably not needed)
		return -1;
	}

	// Step 1: Read dir_inode's data block and check each directory entry of dir_inode
	// Step 2: Check if fname (directory name) is already used in other entries
	for(int i = 0; i < ((int)ceil((double)dir_inode.size / (double)BLOCK_SIZE)) /* number of blocks */; i++){
		int blockNum = dir_inode.direct_ptr[i];
		if(blockNum == -1 || i > 15){
			// This is probably impossible (mostly for sanity check)
			break;
		}
		struct dirent* currBlock = (struct dirent*)calloc(1, BLOCK_SIZE);
		bio_read(blockNum, currBlock);
		for(int j = 0; j < ((int)floor((double)BLOCK_SIZE / (double)sizeof(struct dirent))); j++){
			if((currBlock+j)->valid == 1){
				if(strcmp(fname, (currBlock+j)->name) == 0){
					// Duplicate dirent name
					free(currBlock);
					return -1;
				}
			}
		}
		free(currBlock);
	}
	// Step 3: Add directory entry in dir_inode's data block and write to disk
	int newEntry = 0;
	for(int i = 0; i < 16; i++){
		if(dir_inode.direct_ptr[i] == -1){
			// Allocate a new data block for this directory if it does not exist
			int check = get_avail_blkno();
			if(check == -1){
				// Not enough available data blocks
				return -1;
			}
			dir_inode.direct_ptr[i] = check;
			struct dirent* newBlock = (struct dirent*)calloc(1, BLOCK_SIZE);
			bio_write(check, newBlock);
			if(i < 15){
				dir_inode.direct_ptr[i+1] = -1;
			}
			dir_inode.vstat.st_blocks++; // DO I NEED TO KEEP TRACK????????? Yes
			free(newBlock);
		}
		struct dirent* curr = (struct dirent*)calloc(1, BLOCK_SIZE);
		bio_read(dir_inode.direct_ptr[i], curr);
		for(int j = 0; j < ((int)floor((double)BLOCK_SIZE / (double)sizeof(struct dirent))); j++){
			if((curr+j)->valid == 0){
				(curr+j)->valid = 1;
				(curr+j)->ino = f_ino;
				(curr+j)->len = name_len;
				strcpy((curr+j)->name, fname);
				
				// Write directory entry
				bio_write(dir_inode.direct_ptr[i], curr);
				newEntry = 1;
				break;
			}
		}
		free(curr);
		if(newEntry == 1){
			break;
		}

	}
	if(newEntry == 0){
		// No more available direct ptrs
		return -1;
	}
	
	// Update directory inode
	struct inode* update = (struct inode*)calloc(1, sizeof(struct inode));
	*update = dir_inode;
	update->size += sizeof(struct dirent);
	update->vstat.st_size += sizeof(struct dirent);
	update->vstat.st_atime = time(NULL);
	update->vstat.st_mtime = time(NULL);

	writei(update->ino, update);
	free(update);

	return 0;
}

int dir_remove(struct inode dir_inode, const char *fname, size_t name_len) {

	if(dir_inode.type == (1)){ // Makes sure the inode only contains ptrs to dirents (Probably not needed)
		return -1;
	}
	// Step 1: Read dir_inode's data block and checks each directory entry of dir_inode
	// Step 2: Check if fname exist
	for(int i = 0; i < ((int)ceil((double)dir_inode.size / (double)BLOCK_SIZE)) /* number of blocks */; i++){
		int blockNum = dir_inode.direct_ptr[i];
		if(blockNum == -1 || i > 15){
			// This is probably impossible (mostly for sanity check)
			return -1;
		}
		struct dirent* currBlock = (struct dirent*)calloc(1, BLOCK_SIZE);
		bio_read(blockNum, currBlock);
		for(int j = 0; j < ((int)floor((double)BLOCK_SIZE / (double)sizeof(struct dirent))); j++){
			if((currBlock+j)->valid == 1){
				if(strcmp(fname, (currBlock+j)->name) == 0){
					// Dirent name found
					// Step 3: If exist, then remove it from dir_inode's data block and write to disk
					(currBlock+j)->valid = 0;
					bio_write(blockNum, currBlock);

					// Update directory inode
					struct inode* update = (struct inode*)calloc(1, sizeof(struct inode));
					*update = dir_inode;
					update->size -= sizeof(struct dirent);
					update->vstat.st_size -= sizeof(struct dirent);
					update->vstat.st_atime = time(NULL);
					update->vstat.st_mtime = time(NULL);
					writei(dir_inode.ino, update);

					free(currBlock);
					free(update);
					return 1;
				}
			}
		}
		free(currBlock);
	}
	return -1;
}

/* 
 * namei operation
 */
int get_node_by_path(const char *path, uint16_t ino, struct inode *inode) {
	
	// Step 1: Resolve the path name, walk through path, and finally, find its inode.
	// Note: You could either implement it in a iterative way or recursive way
	
	if(!path){
		return -1;
	}
	char* copy = (char*)malloc(strlen(path)+1);
	strcpy(copy, path);
	const char s[2] = "/";
	char* token = strtok(copy, s);
	struct dirent* curr = (struct dirent*)calloc(1, sizeof(struct dirent));
	curr->ino = 0;
	while(token != NULL){
		int check = dir_find(curr->ino, token, strlen(token), curr);
		if(check == -1){
			return -1;
		}
		
		token = strtok(NULL, s);
	}
	readi(curr->ino, inode);
	free(copy);
	return 1;
}

/* 
 * Make file system
 */

/*
 * returns a blank stat structure
*/
struct stat* getStat(uint16_t ino){
	struct stat* result = (struct stat*)malloc(sizeof(struct stat));
	result->st_ino = ino;
	result->st_uid = getuid();
	result->st_gid = getgid();
	result->st_atime = time(NULL);
	result->st_mtime = time(NULL);

	return result;
}

/*
 * Gets a new struct stat directory
 * Does not set st_size, st_blksize, st_blocks
*/
struct stat* getStatDir(uint16_t ino){
	struct stat* result = getStat(ino);
	result->st_mode = S_IFDIR | 0755;
	result->st_nlink = 2;
	return result;
}

/*
 * Gets a new struct stat directory
 * Does not set st_size, st_blksize, st_blocks
*/
struct stat* getStatFile(uint16_t ino){
	struct stat* result = getStat(ino);
	result->st_mode = S_IFREG | 0644;
	result->st_nlink = 1;
	return result;
}



int tfs_mkfs() {

	// Call dev_init() to initialize (Create) Diskfile
	dev_init(diskfile_path);
	// write superblock information
	superBlock = (struct superblock*)calloc(1, BLOCK_SIZE);
	superBlock->magic_num = MAGIC_NUM;
	superBlock->max_inum = MAX_INUM;
	superBlock->max_dnum = MAX_DNUM;
	superBlock->i_bitmap_blk = 1;
	superBlock->d_bitmap_blk = 2;
	superBlock->i_start_blk = 3;
	superBlock->d_start_blk = 3 + (int)ceil((double) (MAX_INUM * sizeof(struct inode)) / (double) BLOCK_SIZE);
	bio_write(0, superBlock);
	// initialize inode bitmap
	inodeBitmap = (bitmap_t)calloc(1, BLOCK_SIZE);
	// initialize data block bitmap
	dataBitmap = (bitmap_t)calloc(1, BLOCK_SIZE);
	// update bitmap information for root directory
	set_bitmap(inodeBitmap, 0); 
	set_bitmap(dataBitmap, 0); // NEED TO SET DATA BITMAP TO ACOMIDATE FOR SUPER BLOCK AND ETC I GUESS THIS COULD BE A WAY TO CHECK IF THERE IS ENOUGHT ROOM ON THE DISK
	bio_write(superBlock->i_bitmap_blk, inodeBitmap);
	bio_write(superBlock->d_bitmap_blk, dataBitmap);
	// update inode for root directory
	struct inode* dir = (struct inode*)calloc(1, BLOCK_SIZE);
	dir->ino = 0; // Inode will not start at 0 I think......... idk wait isn't MAX DNUM only for data blocks.... does that include the superblocks and the bitmaps????
	dir->valid = 1;
	dir->size = sizeof(struct dirent)*2;
	dir->type = 0;
	dir->link = 0;
	dir->direct_ptr[0] = superBlock->d_start_blk;
	dir->direct_ptr[1] = -1;
	struct stat* dirStat = getStatDir(0);
	dirStat->st_size = sizeof(struct dirent)*2;
	dirStat->st_blksize = BLOCK_SIZE; // DO I NEED TO KEEP TRACK?????????
	dirStat->st_blocks = 1; // DO I NEED TO KEEP TRACK?????????
	dir->vstat = *dirStat;
	bio_write(superBlock->i_start_blk, dir); 

	free(dir);
	free(dirStat);
	
	// Add . and .. to the root directory necessary??? Yes
	struct dirent* dirBlock = (struct dirent*)calloc(1, BLOCK_SIZE);
	dirBlock->ino = 0;
	dirBlock->valid = 1;
	strcpy(dirBlock->name, "."); // Just a reference to itself
	dirBlock->len = strlen(".")+1;
	(dirBlock+1)->ino = 0; // Assuming that the root directory's parent is the root directory
	(dirBlock+1)->valid = 1;
	strcpy((dirBlock+1)->name, "..");
	(dirBlock+1)->len = strlen("..")+1;
	bio_write(superBlock->d_start_blk, dirBlock);

	free(dirBlock);
	//debug(0);
	return 0;
}


/* 
 * FUSE file operations
 */
static void *tfs_init(struct fuse_conn_info *conn) {
	pthread_mutex_lock(&mutex);
	// Step 1a: If disk file is not found, call mkfs
	if(dev_open(diskfile_path) != 0){
		tfs_mkfs();
	}else{
		// Step 1b: If disk file is found, just initialize in-memory data structures
		// and read superblock from disk
		superBlock = (struct superblock*)calloc(1, BLOCK_SIZE);
		inodeBitmap = (bitmap_t)calloc(1, BLOCK_SIZE);
		dataBitmap = (bitmap_t)calloc(1, BLOCK_SIZE);
		bio_read(0, superBlock);
		bio_read(superBlock->i_bitmap_blk, inodeBitmap);
		bio_read(superBlock->d_bitmap_blk, dataBitmap);

	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

static void tfs_destroy(void *userdata) {
	pthread_mutex_lock(&mutex);
	/*
	int numBlockUsed = 0;
	for(int i = 0; i < MAX_DNUM; i++){
		if(get_bitmap(dataBitmap, i) == 1){
			numBlockUsed++;
		}
	}
	*/
	//printf("BLOCKED USED: %d\n", numBlockUsed);

	// Step 1: De-allocate in-memory data structures
	free(superBlock);
	free(inodeBitmap);
	free(dataBitmap);
	// Step 2: Close diskfile
	dev_close();
	pthread_mutex_unlock(&mutex);
}

static int tfs_getattr(const char *path, struct stat *stbuf) {
	pthread_mutex_lock(&mutex);
	//printf("Getattr - path: %s\n", path);
	// Step 1: call get_node_by_path() to get inode from path
	struct inode* statGet = (struct inode*)calloc(1, sizeof(struct inode));
	int check = get_node_by_path(path, 0, statGet);
	if(check == -1){
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return -ENOENT;
	}
	// Step 2: fill attribute of file into stbuf from inode
	//printf("2\n");
	*stbuf = statGet->vstat;
	//printf("2\n");
	pthread_mutex_unlock(&mutex);
	return 0;
}

static int tfs_opendir(const char *path, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutex);
	//printf("Opendir - path: %s\n", path);
	// Step 1: Call get_node_by_path() to get inode from path
	struct inode* openDir = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(path, 0, openDir);
	if(check == -1){
		// Step 2: If not find, return -1
		free(openDir);
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}else{
		if(openDir->valid == -1){ // Not sure if this is necessary (just for my sanity I guess)
			// Step 2: If not find, return -1
			free(openDir);
			//printf("2\n");
			pthread_mutex_unlock(&mutex);
			return -1;
		}
	}
	free(openDir);
	//printf("3\n");
	pthread_mutex_unlock(&mutex);
    return 0;
}

static int tfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutex);
	//printf("Readdir - path: %s\n", path);
	// Step 1: Call get_node_by_path() to get inode from path
	struct inode* readDir = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(path, 0, readDir);
	if(check == -1){
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 2: Read directory entries from its data blocks, and copy them to filler
	for(int i = 0; i < ((int)ceil((double)readDir->size / (double)BLOCK_SIZE)) /* Number of blocks */; i++){
		int blockNum = readDir->direct_ptr[i];
		if(blockNum == -1 || i > 15){
			// This is probably impossible (mostly for sanity check)
			free(readDir);
			//printf("2\n");
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		struct dirent* currBlock = (struct dirent*)calloc(1, BLOCK_SIZE);
		bio_read(blockNum, currBlock);
		for(int j = 0; j < ((int)floor((double)BLOCK_SIZE / (double)sizeof(struct dirent))) /* Number of dirents in a block */; j++){
			if((currBlock+j)->valid == 1){
				struct inode* currInode = (struct inode*)malloc(sizeof(struct inode));
				readi((currBlock+j)->ino, currInode);
				filler(buffer, (currBlock+j)->name, &currInode->vstat, 0);
				free(currInode);
			}
		}
		free(currBlock);
	}
	free(readDir);
	//printf("3\n");
	pthread_mutex_unlock(&mutex);
	return 0;
}

static int tfs_mkdir(const char *path, mode_t mode) {
	pthread_mutex_lock(&mutex);
	//debug(0);
	//printf("Mkdir - path: %s\n", path);
	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name
	char* copyDir = (char*)malloc(strlen(path)+1);
	char* copyBase = (char*)malloc(strlen(path)+1);
	strcpy(copyDir, path);
	strcpy(copyBase, path);
	char* dirName = dirname(copyDir);
	char* baseName = basename(copyBase);
	// Step 2: Call get_node_by_path() to get inode of parent directory
	struct inode* parentDir = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(dirName, 0, parentDir);
	if(check == -1){
		free(copyDir);
		free(copyBase);
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 3: Call get_avail_ino() to get an available inode number
	int inodeNum = get_avail_ino();
	if(inodeNum == -1){
		free(copyDir);
		free(copyBase);
		//printf("2\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 4: Call dir_add() to add directory entry of target directory to parent directory
	// Increments number of links
	parentDir->vstat.st_nlink++;
	parentDir->link++;
	check = dir_add(*parentDir, inodeNum, baseName, strlen(baseName));
	if(check == -1){
		free(copyDir);
		free(copyBase);
		//printf("3\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}

	// Step 5: Update inode for target directory
	int blockNum = get_avail_blkno();
	if(blockNum == -1){
		free(copyDir);
		free(copyBase);
		//printf("4\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	//printf("inode - %d, block - %d\n", inodeNum, blockNum);
	//debug(0);
	struct inode* targetInode = (struct inode*)malloc(sizeof(struct inode));
	targetInode->ino = inodeNum;
	targetInode->valid = 1;
	targetInode->size = sizeof(struct dirent)*2;
	targetInode->type = 0;
	targetInode->direct_ptr[0] = blockNum;
	targetInode->direct_ptr[1] = -1;
	targetInode->link = 2;
	struct stat* vStat = getStatDir(inodeNum);
	vStat->st_blksize = BLOCK_SIZE; // AGAIN DO I NEED TO HAVE THIS???
	vStat->st_blocks = 1; // AGAIN DO I NEED TO HAVE THIS????
	targetInode->vstat = *vStat;
	// Step 6: Call writei() to write inode to disk
	writei(inodeNum, targetInode);
	//debug(0);
	// Add . and .. to the directory (this is definitely necessary)
	struct dirent* dir = (struct dirent*)calloc(1, BLOCK_SIZE);
	dir->ino = inodeNum;
	dir->valid = 1;
	strcpy(dir->name, ".");
	dir->len = strlen(".")+1;
	(dir+1)->ino = parentDir->ino;
	(dir+1)->valid = 1;
	strcpy((dir+1)->name, "..");
	(dir+1)->len = strlen("..")+1;
	bio_write(targetInode->direct_ptr[0], dir);
	//debug(0);
	free(dir);
	free(vStat);
	free(copyDir);
	free(copyBase);
	//printf("5\n");
	//debug(0);
	pthread_mutex_unlock(&mutex);
	return 0;
}

static int tfs_rmdir(const char *path) {
	pthread_mutex_lock(&mutex);
	//printf("Rmdir\n");
	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name
	char* copyDir = (char*)malloc(strlen(path)+1);
	char* copyBase = (char*)malloc(strlen(path)+1);
	strcpy(copyDir, path);
	strcpy(copyBase, path);
	char* parentName = dirname(copyDir);
	char* baseName = basename(copyBase);
	// Step 2: Call get_node_by_path() to get inode of target directory
	struct inode* rmDir = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(path, 0, rmDir);
	if(check == -1){
		free(copyDir);
		free(copyBase);
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 3: Clear data block bitmap of target directory
	if(rmDir->type != (0)){
		// Making sure that the given path is a directory (Probably not necessary)
		free(copyDir);
		free(copyBase);
		//printf("2\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	if(rmDir->type == (0) && rmDir->size > (sizeof(struct dirent)*2)){
		// Check if there are more than . and .. directories in the target directory
		free(copyDir);
		free(copyBase);
		//printf("3\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	for(int i = 0; i < 16; i++){
		if(rmDir->direct_ptr[i] == -1){
			break;
		}
		unset_bitmap(dataBitmap, rmDir->direct_ptr[i] - superBlock->d_start_blk);
		rmDir->direct_ptr[i] = -1; // Probably not necessary
	}
	bio_write(superBlock->d_bitmap_blk, dataBitmap);
	// Step 4: Clear inode bitmap and its data block
	rmDir->valid = 0;
	writei(rmDir->ino, rmDir);

	unset_bitmap(inodeBitmap, rmDir->ino);
	bio_write(superBlock->i_bitmap_blk, inodeBitmap);

	// Step 5: Call get_node_by_path() to get inode of parent directory
	struct inode* parentDir = (struct inode*)malloc(sizeof(struct inode));
	check = get_node_by_path(parentName, 0, parentDir);
	if(check == -1){ // Literally impossible (purely for sanity)
		free(parentDir);
		free(rmDir);
		free(copyDir);
		free(copyBase);
		//printf("4\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 6: Call dir_remove() to remove directory entry of target directory in its parent directory
	parentDir->vstat.st_nlink--;
	parentDir->link--;
	check = dir_remove(*parentDir, baseName, strlen(baseName));
	if(check == -1){
		free(parentDir);
		free(rmDir);
		free(copyDir);
		free(copyBase);
		//printf("5\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	free(parentDir);
	free(rmDir);
	free(copyDir);
	free(copyBase);
	//printf("6\n");
	pthread_mutex_unlock(&mutex);
	return 0;
}

static int tfs_releasedir(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutex);
	//printf("Create\n");
	// Step 1: Use dirname() and basename() to separate parent directory path and target file name
	char* copyDir = (char*)malloc(strlen(path)+1);
	char* copyBase = (char*)malloc(strlen(path)+1);
	strcpy(copyDir, path);
	strcpy(copyBase, path);
	char* parentName = dirname(copyDir);
	char* targetName = basename(copyBase);
	// Step 2: Call get_node_by_path() to get inode of parent directory
	struct inode* parentInode = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(parentName, 0, parentInode);
	if(check == -1){
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 3: Call get_avail_ino() to get an available inode number
	int inodeNum = get_avail_ino();
	if(inodeNum == -1){
		//printf("2\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 4: Call dir_add() to add directory entry of target file to parent directory
	dir_add(*parentInode, inodeNum, targetName, strlen(targetName));

	// Step 5: Update inode for target file
	/*
	int blockNum = get_avail_blkno();
	if(blockNum == -1){
		//printf("3\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	*/
	struct inode* targetFile = (struct inode*)malloc(sizeof(struct inode));
	targetFile->ino = inodeNum;
	targetFile->valid = 1;
	targetFile->size = 0;
	targetFile->type = 1;
	targetFile->link = 1;
	//targetFile->direct_ptr[0] = blockNum;
	targetFile->direct_ptr[0] = -1;
	struct stat* vStat = getStatFile(inodeNum);
	vStat->st_size = 0;
	vStat->st_blksize = BLOCK_SIZE;
	vStat->st_blocks = 0;
	targetFile->vstat = *vStat;

	// Step 6: Call writei() to write inode to disk
	writei(inodeNum, targetFile);

	free(vStat);
	free(targetFile);
	free(copyDir);
	free(copyBase);
	//printf("4\n");
	pthread_mutex_unlock(&mutex);
	return 0;
}

static int tfs_open(const char *path, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutex);
	//printf("Open\n");
	// Step 1: Call get_node_by_path() to get inode from path
	struct inode* inodeOpen = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(path, 0, inodeOpen);
	if(check == -1){
		// Step 2: If not find, return -1
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	if(inodeOpen->valid == 0){ // Check if the inode is valid (probably not necessary)
		//printf("2\n");
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	//printf("3\n");
	pthread_mutex_unlock(&mutex);
	return 0;
}

static int tfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutex);
	//printf("Read\n");
	// Step 1: You could call get_node_by_path() to get inode from path
	struct inode* targetInode = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(path, 0, targetInode);
	if(check == -1){
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	// Step 2: Based on size and offset, read its data blocks from disk
	if(targetInode->type != (1)){ // Makes sure it's a file
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	int startBlock = (int)floor((double)offset / (double)BLOCK_SIZE);
	int numBytes = 0;
	int indexCount = offset - (startBlock*BLOCK_SIZE);
	for(int i = startBlock; i < targetInode->vstat.st_blocks; i++){
		if(targetInode->direct_ptr[i] == -1 || i > 15){
			// This is probably impossible (mostly for sanity check)
			break;
		}

		char* data = (char*)calloc(1, BLOCK_SIZE);
		bio_read(targetInode->direct_ptr[i], data);
		for(int j = indexCount; j < BLOCK_SIZE; j++){
			if(numBytes >= size){
				free(data);
				free(targetInode);
				pthread_mutex_unlock(&mutex);
				return numBytes;
			}
			// Step 3: copy the correct amount of data from offset to buffer
			*(buffer+numBytes) = *(data+j);
			numBytes++;
		}
		indexCount = 0;
		free(data);
	}
	free(targetInode);
	// Note: this function should return the amount of bytes you copied to buffer
	pthread_mutex_unlock(&mutex);
	return numBytes;
}

/*
 * 
*/
static int tfs_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutex);
	//printf("Write %s - %ld %ld\n", path, size, offset);
	
	// Step 1: You could call get_node_by_path() to get inode from path
	struct inode* targetInode = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(path, 0, targetInode);
	if(check == -1){
		free(targetInode);
		//printf("1\n");
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	//printf("Stat size: %ld\n", targetInode->vstat.st_size);
	// Step 2: Based on size and offset, read its data blocks from disk
	if(targetInode->type != (1)){ // Makes sure it's a file
		free(targetInode);
		//printf("2\n");
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	//printf("blocks - %ld\n", targetInode->vstat.st_blocks);
	// Allocates more blocks if needed
	for(int i = 0; i < (int)ceil((double)(offset+size) / (double)BLOCK_SIZE); i++){
		if(i > 15){
			// If the size and offset exceeds the file size limit
			break;
		}
		if(targetInode->direct_ptr[i] == -1){
			int check = get_avail_blkno();
			if(check == -1){
				// Not enough available data blocks but will try to write as much information to the given allocated blocks
				break;
			}
			targetInode->direct_ptr[i] = check;
			struct dirent* newBlock = (struct dirent*)calloc(1, BLOCK_SIZE); // Clears the data block but not necessary
			bio_write(check, newBlock);
			if(i < 15){
				targetInode->direct_ptr[i+1] = -1;
			}
			targetInode->vstat.st_blocks++;
			free(newBlock);
		}
	}
	//printf("blocks - %ld\n", targetInode->vstat.st_blocks);
	int numBytes = 0;
	int startBlock = (int)floor((double)offset / (double)BLOCK_SIZE);
	int indexCount = offset - (startBlock*BLOCK_SIZE);
	//printf("Startblock %d\n", startBlock);
	// Need to check if the blocks can be allocated ^^^
	for(int i = startBlock; i < targetInode->vstat.st_blocks; i++){
		if(targetInode->direct_ptr[i] == -1){
			// Literally impossible but it might happen (sanity check)
			free(targetInode);
			//printf("3\n");
			pthread_mutex_unlock(&mutex);
			return numBytes;
		}
		char* data = (char*)calloc(1, BLOCK_SIZE);
		bio_read(targetInode->direct_ptr[i], data);
		for(int j = indexCount; j < BLOCK_SIZE; j++){
			if(numBytes >= size){
				break;
			}
			// Step 3: copy the correct amount of data from offset to buffer
			*(data+j) = *(buffer+numBytes);
			numBytes++;
			targetInode->size++;
			targetInode->vstat.st_size++;
			
		}
		bio_write(targetInode->direct_ptr[i], data);
		indexCount = 0;
		free(data);
		if(numBytes >= size){
			break;
		}
	}
	// Step 4: Update the inode info and write it to disk
	//printf("Stat size: %ld\n", targetInode->vstat.st_size);
	writei(targetInode->ino, targetInode);
	free(targetInode);
	//printf("4 - bytes: %d\n", numBytes);
	// Note: this function should return the amount of bytes you write to disk
	pthread_mutex_unlock(&mutex);
	return numBytes;
}

static int tfs_unlink(const char *path) {
	pthread_mutex_lock(&mutex);
	//printf("Unlink\n");
	// Step 1: Use dirname() and basename() to separate parent directory path and target file name
	char* copyDir = (char*)malloc(strlen(path)+1);
	char* copyBase = (char*)malloc(strlen(path)+1);
	strcpy(copyDir, path);
	strcpy(copyBase, path);
	char* parentName = dirname(copyDir);
	char* targetName = basename(copyBase);
	// Step 2: Call get_node_by_path() to get inode of target file
	struct inode* targetInode = (struct inode*)malloc(sizeof(struct inode));
	int check = get_node_by_path(path, 0, targetInode);
	if(check == -1){
		free(targetInode);
		free(copyDir);
		free(copyBase);
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 3: Clear data block bitmap of target file
	for(int i = 0; i < 16; i++){
		int blockNum = targetInode->direct_ptr[i];
		if(blockNum == -1){
			break;
		}
		unset_bitmap(dataBitmap, blockNum - superBlock->d_start_blk);
		targetInode->direct_ptr[i] = -1; // Probably not necessary
	}
	bio_write(superBlock->d_bitmap_blk, dataBitmap);
	// Step 4: Clear inode bitmap and its data block
	targetInode->valid = 0;
	writei(targetInode->ino, targetInode);

	unset_bitmap(inodeBitmap, targetInode->ino);
	bio_write(superBlock->i_bitmap_blk, inodeBitmap);
	// Step 5: Call get_node_by_path() to get inode of parent directory
	struct inode* parentDir = (struct inode*)malloc(sizeof(struct inode));
	check = get_node_by_path(parentName, 0, parentDir);
	if(check == -1){ // Literally impossible (purly for sanity)
		free(parentDir);
		free(targetInode);
		free(copyDir);
		free(copyBase);
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// Step 6: Call dir_remove() to remove directory entry of target file in its parent directory
	check = dir_remove(*parentDir, targetName, strlen(targetName));
	if(check == -1){
		free(parentDir);
		free(targetInode);
		free(copyDir);
		free(copyBase);
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	free(parentDir);
	free(copyDir);
	free(copyBase);
	free(targetInode);
	pthread_mutex_unlock(&mutex);
	return 0;
}

static int tfs_truncate(const char *path, off_t size) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_release(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
	return 0;
}

static int tfs_flush(const char * path, struct fuse_file_info * fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_utimens(const char *path, const struct timespec tv[2]) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}


static struct fuse_operations tfs_ope = {
	.init		= tfs_init,
	.destroy	= tfs_destroy,

	.getattr	= tfs_getattr,
	.readdir	= tfs_readdir,
	.opendir	= tfs_opendir,
	.releasedir	= tfs_releasedir,
	.mkdir		= tfs_mkdir,
	.rmdir		= tfs_rmdir,

	.create		= tfs_create,
	.open		= tfs_open,
	.read 		= tfs_read,
	.write		= tfs_write,
	.unlink		= tfs_unlink,

	.truncate   = tfs_truncate,
	.flush      = tfs_flush,
	.utimens    = tfs_utimens,
	.release	= tfs_release
};


int main(int argc, char *argv[]) {
	int fuse_stat;

	getcwd(diskfile_path, PATH_MAX);
	strcat(diskfile_path, "/DISKFILE");

	fuse_stat = fuse_main(argc, argv, &tfs_ope, NULL);

	return fuse_stat;
}
