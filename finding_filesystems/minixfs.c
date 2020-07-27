/**
* Finding Filesystems Lab
* CS 241 - Fall 2018
*/

#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string, "Free blocks: %zd\n"
                            "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!

    inode* val = get_inode(fs,path);
    struct stat i;
    int status  = minixfs_stat(fs,char,&i);

    if (!val){
      if (status==-1)
      errno =  ENOENT;

      return -1;
    }
    else{
    uint16_t mode = val->mode;
    int type = mode>>  RWX_BITS_NUMBER;
    mode = new_permissions&0x3ff;
    type = type<<RWX_BITS_NUMBER;
    mode = mode|type;
    val->mode = mdoe;
    return clock_gettime(CLOCK_REALTIME,&val->ctime);
  }
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    struct stat i;
    int status  = minixfs_stat(fs,char,&i);

    inode* val = get_inode(fs,path);
    if (!val){
      if (status ==-1)
      errno =  ENOENT;

      return -1;
    }
    else{
      if (val->uid != (uid_t)-1){
      val->uid = owner;
    }
    if (val->gid != (uid_t)-1){
      val->gid = group;
    }
    return clock_gettime(CLOCK_REALTIME,&val->ctime);
  }
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode* val = get_inode(fs,char);
    if (val){
      clock_gettime(CLOCK_REALTIME,&val->ctime);
      return NULL;
    }
    if (!valid_filename(path)){
      return NULL;
    }
    inode_number i = first_unused_inode(fs);
    inode* first = fs->inode_root+i;

    const char *filename;
    inode* parent = parent_directory(fs,path,filename); // is this works? take pointer
    if (!is_directory(parent)){
      perror("The parent is not correct");
      return NULL;
    }
    init_inode(parent,first);

    int len = strlen(path);
     const char *endptr = path + len;
     while (*endptr != '/') {
         endptr--;
     }
    char *parent_path = malloc(endptr - path + strlen("/") + 1);
   strncpy(parent_path, path, endptr - path + 1);
   parent_path[endptr - path + 1] = '\0';
off_t parent_size = (off_t)(parent->size);
   ssize_t cond = minixfs_write(fs,(const char*)parent_path,filename,MAX_DIR_NAME_LEN,&parent_size);

    return first;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
    }
    // TODO implement your own virtual file here

}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
  size_t need_writtern = count;
  unsigned long needed_blocks = (count + *off) / (16 * KILOBYTE);
  if((count + *off) % (16 * KILOBYTE) != 0) needed_blocks = (count + *off+16*KILOBYTE-1) / (16 * KILOBYTE);
  if(needed_blocks > NUM_DIRECT_INODES + NUM_INDIRECT_INODES){
    errno = ENOSPC;
    return -1;
  }
  if(minixfs_min_blockcount(fs, path, needed_blocks) == -1){
    errno = ENOSPC;
    return -1;
  }

 inode *target = get_inode(fs, path);
 int data_index = *off / (16 * KILOBYTE);
 int completed = 0;
 if(data_index < NUM_DIRECT_INODES){

  int remains = *off % (16 * KILOBYTE);
  for(int i = data_index; i < NUM_DIRECT_INODES; i++){
    struct stat temp;
    int cond = minixfs_stat(fs,path,&temp);
    if (cond==-1){

    }


   size_t finished = 16 * KILOBYTE - remains;
   if(count < 16 * KILOBYTE) {
     memcpy(&fs->data_root[target->direct[i]].data[remains], buf + completed, count);
         completed += count;
      remains = 0;
      count -= count;
      break;

      }
   else {
     memcpy(&fs->data_root[target->direct[i]].data[remains], buf + completed, finished);
     completed += finished;
     remains = 0;
     count -= finished;
   }
  }
  if(count > 0){ // still need to write but no direct data block left
   data_block indir = fs->data_root[target->indirect];

   for(int i = 0; i < (int)NUM_INDIRECT_INODES; i++){
    size_t finished = 16 * KILOBYTE - remains;
    int total_indirect_blocks = indir.data[i * 4];
    if(count > 16 * KILOBYTE) {
          memcpy(&fs->data_root[total_indirect_blocks].data[0], buf + completed, finished);
          completed += finished;
          count =  count - finished;

        }
    else {
     memcpy(&fs->data_root[total_indirect_blocks].data[0], buf + completed, count);
          completed += count;
          count =0;
     break;
    }
   }
  }
 }

 else{
  int remain = *off - NUM_DIRECT_INODES * 16 *KILOBYTE;
  data_block indir = fs->data_root[target->indirect];
  int new_dir_index = remain / (16 * KILOBYTE);

    int remains = remain % (16 * KILOBYTE);

    for(int i = new_dir_index; i < (int)NUM_INDIRECT_INODES; i++){
      size_t finished = 16 * KILOBYTE - remains;
      int total_indirect_blocks = indir.data[i * 4];
      if(count > 16 * KILOBYTE) {
        memcpy(&fs->data_root[total_indirect_blocks].data[remains], buf + completed, finished);
        completed += finished;
        remains = 0;
        count -= finished;
      }
      else {
        memcpy(&fs->data_root[total_indirect_blocks].data[remains], buf + completed, count);
        completed += count;
        count =0;
        break;
      }

    }
 }


  if (*off+need_writtern>target->size){
    target->size = *off+need_writtern;
  }

  if(clock_gettime(CLOCK_REALTIME, &target->mtim) == -1){
    return -1;
  }
  if(clock_gettime(CLOCK_REALTIME, &target->atim) == -1){
    return -1;
  }
    *off += completed;
    return completed;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    inode *target = get_inode(fs, path);
    if(target == NULL) {
      errno = ENOENT;
      return -1;
    }
    size_t targetsize = target->size;
    if(*off > (long)targetsize) return 0;
    ssize_t needs_reading ;
    if (count<targetsize-*off){
        needs_reading = count;
    }
    else{
      needs_reading = targetsize-*off;
    }
    int data_index = *off / (16 * KILOBYTE);
    int completed = 0;

    if(data_index < NUM_DIRECT_INODES){

      int remains = *off % (16 * KILOBYTE);
      for(int i = data_index; i < NUM_DIRECT_INODES; i++){

        int finished = 16 * KILOBYTE - remains;
        if(needs_reading > 16 * KILOBYTE) {
          memcpy(buf + completed, &fs->data_root[target->direct[i]].data[remains], (size_t)finished);
          completed += finished;
          remains = 0;
          needs_reading -= (ssize_t)finished;

        }
        else {

          memcpy(buf + completed, &fs->data_root[target->direct[i]].data[remains], needs_reading);

          completed += needs_reading;
          remains = 0;
          needs_reading =0;
          break;
        }

      }

      if(needs_reading > 0){

        data_block indir = fs->data_root[target->indirect];

        for(int i = 0; i < (int)NUM_INDIRECT_INODES; i++){
          size_t finished = 16 * KILOBYTE/* - remains*/;
          int total_indirect_blocks = indir.data[i * 4];
          if(needs_reading > 16 * KILOBYTE) {
            memcpy(buf + completed, &fs->data_root[total_indirect_blocks].data[0], finished);
            completed += finished/* - remains*/;

            needs_reading -= finished;
          }
          else {
            memcpy(buf + completed, &fs->data_root[total_indirect_blocks].data[0], needs_reading);
            completed += needs_reading;

            needs_reading =0;
            break;
          }
        }
      }
    }
    else{
      int remain = *off - NUM_DIRECT_INODES * 16 *KILOBYTE;
      data_block indir = fs->data_root[target->indirect];
      int new_dir_index = remain / (16 * KILOBYTE);

      int remains = remain % (16 * KILOBYTE);

      for(int i = new_dir_index; i < (int)NUM_INDIRECT_INODES; i++){
        size_t finished = 16 * KILOBYTE - remains;
        int total_indirect_blocks = indir.data[i * 4];
        if(needs_reading > 16 * KILOBYTE) {
          memcpy(buf + completed, &fs->data_root[total_indirect_blocks].data[remains], finished);
          completed += finished;
          remains = 0;
          needs_reading -= (ssize_t)finished;
        }
        else {
          memcpy(buf + completed, &fs->data_root[total_indirect_blocks].data[remains], needs_reading);
          completed += needs_reading;
          needs_reading =0;
          break;
        }

      }
    }
    if(clock_gettime(CLOCK_REALTIME, &target->atim) == -1){
      return -1;
    }
    *off += completed;
    return completed;

}
