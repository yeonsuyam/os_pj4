#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44
#define DIRECT_CNT 100
#define INDIRECT_CNT 128
#define DOUBLY_CNT 16384


struct list cache_list;
/* On-disk inode.
   Must be exactly DISK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    //disk_sector_t start;                /* First data sector. */
    disk_sector_t direct[DIRECT_CNT];
    disk_sector_t indirect;
    disk_sector_t doubly;
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
    uint32_t unused[24];               /* Not used. */
  };

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, DISK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode 
  {
    struct list_elem elem;              /* Element in inode list. */
    disk_sector_t sector;               /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /* Inode content. */
  };

/* Returns the disk sector that contains byte offset POS within
   INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static disk_sector_t
byte_to_sector (const struct inode *inode, off_t pos) 
{
  ASSERT (inode != NULL);
<<<<<<< HEAD
\
  if (pos < inode->data.length){
    int index = pos / DISK_SECTOR_SIZE;

    if(index < DIRECT_CNT)
      return inode->data.direct[index];

    index -= DIRECT_CNT;
=======

  if (pos < inode->data.length){
    int index = pos / DISK_SECTOR_SIZE;


    if(index < DIRECT_CNT)
      return inode->data.direct[index];
    index -= (DIRECT_CNT -1);
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3

    if(index < INDIRECT_CNT){
      disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
      disk_read(filesys_disk, inode->data.indirect, buf);
<<<<<<< HEAD
//      printf("index - 1 : %d index : %d\n", buf[index-1], buf[index]);
      return buf[index];
    }

    index -= DOUBLY_CNT;
=======
      return buf[index -1];
    }

>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
    if (index < DOUBLY_CNT)
    {
      disk_sector_t buf1[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
      disk_read(filesys_disk, inode->data.doubly, buf1);

      index = index/INDIRECT_CNT;
      disk_sector_t buf2[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
<<<<<<< HEAD
      disk_read(filesys_disk, buf1[index], buf2);       
      return buf2[index];
=======
      disk_read(filesys_disk, buf1[index], buf2);

      return buf2[index-1];
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
    }
    else
      ASSERT(0);

  }

  else
    return -1;

}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Initializes the inode module. */
void
inode_init (void) 
{
  list_init (&open_inodes);
  list_init (&cache_list);
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   disk.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (disk_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  //printf("sizeof *disk_inode : %d\n", sizeof *disk_inode);
  ASSERT (sizeof *disk_inode == DISK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      if (sector_allocate(sectors, disk_inode))
      {
        //printf("success\n");
        success = true;
        disk_write (filesys_disk, sector, disk_inode);

      }


      /*
      if (free_map_allocate (sectors, &disk_inode->start))
        {
          disk_write (filesys_disk, sector, disk_inode);
          if (sectors > 0) 
            {
              static char zeros[DISK_SECTOR_SIZE];
              size_t i;
              
              for (i = 0; i < sectors; i++) 
                disk_write (filesys_disk, disk_inode->start + i, zeros); 
            }
          success = true; 
        } 

        */
      free (disk_inode);
    }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (disk_sector_t sector) 
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector) 
        {
          inode_reopen (inode);
          return inode; 
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  disk_read (filesys_disk, inode->sector, &inode->data);
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
<<<<<<< HEAD
  if (inode != NULL){
      inode->open_cnt++;
  }
=======
  if (inode != NULL)
    inode->open_cnt++;
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
  return inode;
}

/* Returns INODE's inode number. */
disk_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode) 
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);
<<<<<<< HEAD
      size_t index = bytes_to_sectors(inode->data.length);
      
=======
 
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
      /* Deallocate blocks if removed. */
      if (inode->removed) 
        {
          free_map_release (inode->sector, 1);
          int i;
<<<<<<< HEAD
          for (i =0; i< min(index, DIRECT_CNT); i++){
            free_map_release(inode->data.direct[i], 1);
           
=======
          size_t index = bytes_to_sectors(inode->data.length);
          for (i =0; i< min(index, DIRECT_CNT); i++){
            free_map_release(inode->data.direct[i], 1);
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
          }

          index -= min(index, DIRECT_CNT);
          if (index > 0)
          {
<<<<<<< HEAD
            indirect_release_temp(index, inode->data.indirect);
=======
            indirect_release(index, inode->data.indirect);
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
          }

          index -= min(index, INDIRECT_CNT);
          if (index > 0)
          {
            disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
            disk_read(filesys_disk, inode->data.doubly, buf);
            int i;
            for (i =0; i < min((index/INDIRECT_CNT +1), INDIRECT_CNT); i++)
            {
<<<<<<< HEAD
              indirect_release_temp(index-(i*INDIRECT_CNT), buf[i]);

=======
              free_map_release(buf[i], 1);
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
              
            }
          }
          
          //free_map_release (inode->data.start,
                            //bytes_to_sectors (inode->data.length)); 
        }
<<<<<<< HEAD
      int i;
      for (i =0; i< min(index, DIRECT_CNT); i++){
        struct cache_entry * ce = is_hit(inode->data.direct[i]);
        if (ce != NULL)
          disk_write(filesys_disk, ce->sector_idx, ce->data);
      }

      index -= min(index, DIRECT_CNT);
      if (index > 0)
      {
        indirect_release(index, inode->data.indirect);
      }

      index -= min(index, INDIRECT_CNT);
      if (index > 0)
      {
        disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
        disk_read(filesys_disk, inode->data.doubly, buf);
        int i;
        for (i =0; i < min((index/INDIRECT_CNT +1), INDIRECT_CNT); i++)
        {
          indirect_release(index-(i*INDIRECT_CNT), buf[i]);

          
        }
      }
=======

>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
      free (inode); 
    }
}

void
indirect_release(size_t index, disk_sector_t indirect)
{
  disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
  disk_read(filesys_disk, indirect, buf);
  int i;
  for (i =0; i < min(index, INDIRECT_CNT); i++)
  {
<<<<<<< HEAD
    struct cache_entry * ce = is_hit(buf[i]);
    if (ce != NULL)
      disk_write(filesys_disk, ce->sector_idx, ce->data);

  }
}

void
indirect_release_temp(size_t index, disk_sector_t indirect)
{
  disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
  disk_read(filesys_disk, indirect, buf);
  int i;
  for (i =0; i < min(index, INDIRECT_CNT); i++)
  {
    free_map_release(buf[i], 1);

=======
    free_map_release(buf[i], 1);
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
  }
}
/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode) 
{
  ASSERT (inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) 
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t *bounce = NULL;

  
  while (size > 0) 
    {
      /* 
      1. cache 에 있는지확인
      2. HIT 그냥 그걸 읽음
      3. MISS if(list_lenght >= 64 이면 FIFO로 evict
      4. disk에서 fetch
      WRITE-BEHIND
      evict함수 안에 dirty bit가 있으면 evict 할때 disk에다가 수정내용을 입력 (mmap할때)
      READ-AHEAD
      하나 fetch할때 그 다음 block도 함께 fetch(읽혀질것 같으니까)
      
      */


      /* Disk sector to read, starting byte offset within sector. */
      disk_sector_t sector_idx = byte_to_sector (inode, offset);
      //printf("sector index : %d\n", sector_idx);
      int sector_ofs = offset % DISK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = DISK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0){
        break;
      }

      struct cache_entry *c = is_hit(sector_idx);
      struct cache_entry *next_c;
      if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE) 
      {
        /* Read full sector directly into caller's buffer. */
        if(c == NULL){
          //FETCH, LIST 추가, MEMCPY
          
          c = new_entry();
          //c = malloc(sizeof *c);
          c -> sector_idx = sector_idx;
          c -> data = malloc (DISK_SECTOR_SIZE);
          ASSERT(c->data != NULL);
  
          disk_read (filesys_disk, sector_idx, c->data);

          list_push_back(&cache_list, &c->elem);
          /*
          next_c = new_entry();
          next_c -> sector_idx = sector_idx+1;
          next_c -> data = malloc (DISK_SECTOR_SIZE);
          disk_read (filesys_disk, sector_idx+1, next_c->data);
          list_push_back(&cache_list, &next_c->elem);
          */

        }
        
        memcpy (buffer + bytes_read, c->data, chunk_size);
        //disk_read (filesys_disk, sector_idx, buffer + bytes_read); 

      }
      else 
      {
        if(c == NULL){
          //FETCH, LIST 추가, MEMCPY
          c = new_entry();
          //c = malloc(sizeof *c);
          c -> sector_idx = sector_idx;
          c -> data = malloc (DISK_SECTOR_SIZE);
          ASSERT(c->data != NULL);
          disk_read (filesys_disk, sector_idx, c->data);
          list_push_back(&cache_list, &c->elem);
          /*
          next_c = new_entry();
          next_c -> sector_idx = sector_idx+1;
          next_c -> data = malloc (DISK_SECTOR_SIZE);
          disk_read (filesys_disk, sector_idx+1, next_c->data);
          list_push_back(&cache_list, &next_c->elem);
          */
        }
        memcpy (buffer + bytes_read, c->data + sector_ofs, chunk_size);
      }
      
      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }
  free (bounce);

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset) 
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;

<<<<<<< HEAD
  ASSERT(inode != NULL);
  if (inode->deny_write_cnt)
    return 0;
//  printf("length : %d", inode->data.length);

  if (offset + size > inode->data.length){
    int temp_sectors = bytes_to_sectors(inode->data.length);
    int last_sectors = bytes_to_sectors(offset + size);
    int cnt = last_sectors - temp_sectors;

//  printf("TEMP SECTOR : %d\n", temp_sectors);

//  printf("LAST SECTOR : %d\n", last_sectors);


  if(temp_sectors < DIRECT_CNT){
    direct_allocate(temp_sectors+1, min(cnt, DIRECT_CNT-temp_sectors), &inode->data);
    
    //현재까지 alloacte한 결과 update
    cnt -= min(cnt, DIRECT_CNT-temp_sectors);
    temp_sectors = min(temp_sectors + cnt, DIRECT_CNT);
  }

  if(DIRECT_CNT <= temp_sectors && temp_sectors < DIRECT_CNT + INDIRECT_CNT){
    temp_sectors = temp_sectors - DIRECT_CNT;
    if(inode->data.indirect == 0)
      if (!free_map_allocate (1, &inode->data.indirect))
        ASSERT(0);

    indirect_allocate(temp_sectors + 1, min(cnt, INDIRECT_CNT-temp_sectors), inode->data.indirect);

    cnt -= min(cnt, INDIRECT_CNT-temp_sectors);
    temp_sectors = min(temp_sectors + cnt, INDIRECT_CNT);
  }
  if(INDIRECT_CNT <= temp_sectors && temp_sectors < INDIRECT_CNT + DOUBLY_CNT){
    temp_sectors = temp_sectors - INDIRECT_CNT;

    if(inode->data.doubly == 0)
      if (!free_map_allocate (1, &inode->data.doubly))
        ASSERT(0);

    doubly_allocate(temp_sectors + 1, min(cnt, DOUBLY_CNT-temp_sectors), inode->data.doubly);

    cnt -= min(cnt, DOUBLY_CNT-temp_sectors);
    temp_sectors = min(temp_sectors + cnt, INDIRECT_CNT);
  }
  if(cnt > 0)
    ASSERT(0);
  inode->data.length = offset + size; 
//  printf("WRITE : %d\n", offset + size); 
  disk_write(filesys_disk, inode->sector, &inode->data);
  }

=======
  if (inode->deny_write_cnt)
    return 0;
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3

  while (size > 0) 
    {
      /* Sector to write, starting byte offset within sector. */
      disk_sector_t sector_idx = byte_to_sector (inode, offset);
<<<<<<< HEAD
      ASSERT(sector_idx != -1);
      int sector_ofs = offset % DISK_SECTOR_SIZE;
=======
      int sector_ofs = offset % DISK_SECTOR_SIZE;

>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = DISK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;
<<<<<<< HEAD
      
=======

>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

<<<<<<< HEAD
      
      struct cache_entry *c = is_hit(sector_idx);
      struct cache_entry *next_c;
      if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE) 
        {

=======
      struct cache_entry *c = is_hit(sector_idx);
      struct cache_entry *next_c;

      if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE) 
        {
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
          /* Write full sector directly to disk. */
          if(c == NULL){
            //FETCH, LIST 추가, MEMCPY
            c = new_entry();
            //c = malloc(sizeof *c);
            c -> sector_idx = sector_idx;
            c -> data = malloc (DISK_SECTOR_SIZE);
            ASSERT(c->data != NULL);
            disk_read (filesys_disk, sector_idx, c->data);
            list_push_back(&cache_list, &c->elem);
            /*
            next_c = new_entry();
          next_c -> sector_idx = sector_idx+1;
          next_c -> data = malloc (DISK_SECTOR_SIZE);
          disk_read (filesys_disk, sector_idx+1, next_c->data);
          list_push_back(&cache_list, &next_c->elem);
          */
<<<<<<< HEAD
          //ASSERT(0);
=======
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
          }
          memcpy (c->data, buffer + bytes_written, chunk_size); 
        }
      else 
        {
          /* We need a bounce buffer. */
<<<<<<< HEAD

=======
         
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
          /* If the sector contains data before or after the chunk
             we're writing, then we need to read in the sector
             first.  Otherwise we start with a sector of all zeros. */
          if(c == NULL){
            //FETCH, LIST 추가, MEMCPY
            c = new_entry();
           // c = malloc(sizeof *c);
            c -> sector_idx = sector_idx;
            c -> data = malloc (DISK_SECTOR_SIZE);
            ASSERT(c->data != NULL);
            if (sector_ofs > 0 || chunk_size < sector_left) 
            disk_read (filesys_disk, sector_idx, c->data);
            else
            memset (c->data, 0, DISK_SECTOR_SIZE);
            list_push_back(&cache_list, &c->elem);
            /*
            next_c = new_entry();
          next_c -> sector_idx = sector_idx+1;
          next_c -> data = malloc (DISK_SECTOR_SIZE);
          
          disk_read (filesys_disk, sector_idx+1, next_c->data);
          list_push_back(&cache_list, &next_c->elem);
          */
          }
<<<<<<< HEAD
=======

>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
          memcpy (c->data + sector_ofs, buffer + bytes_written, chunk_size);
        }

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }
  free (bounce);

  return bytes_written;
}

bool
sector_allocate(size_t sectors, struct inode_disk * disk_inode)
{

  int i;
  for (i =0; i<min(sectors, DIRECT_CNT); i++){
    if(disk_inode->direct[i] == 0)
    {
      if (!free_map_allocate (1, &disk_inode->direct[i])){
        ASSERT(0);
        return false;
      }
      static char zeros[DISK_SECTOR_SIZE];
      disk_write (filesys_disk, disk_inode->direct[i], zeros); 
    }
  }
  sectors -= min(sectors, DIRECT_CNT);
  //printf("sectors : %d\n",sectors);
  if (sectors ==0)
    return true;

  if(!free_map_allocate(1, &disk_inode->indirect))
    return false;
  inode_indirect(min(sectors, INDIRECT_CNT), disk_inode->indirect);
  sectors -= min(sectors, INDIRECT_CNT);

 if (sectors ==0)
    return true;


  if(!free_map_allocate(1, &disk_inode->doubly))
    return false;
  inode_doubly(min(sectors, INDIRECT_CNT), disk_inode);




  if (sectors ==0)
    return true;



}


bool
inode_indirect(size_t sectors, disk_sector_t indirect)
{
  disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
  //memset(&buf, 0, DISK_SECTOR_SIZE);
  int i;
  for(i = 0; i < sectors; i++)
  {

    if (!free_map_allocate (1, &buf[i])){
      return false;
    }


    static char zeros[DISK_SECTOR_SIZE];
    disk_write (filesys_disk, buf[i], zeros); 

  }
  disk_write(filesys_disk, indirect, buf);
  return true;

}


bool 
inode_doubly(size_t sectors, struct inode_disk * disk_inode)
{
  disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
  int i = 0;
  while (sectors >0)
  {

    if (!free_map_allocate (1, &buf[i])){

      return false;
    }

    inode_indirect(min(sectors, INDIRECT_CNT), buf[i]);
    sectors -= min(sectors, INDIRECT_CNT);

    i++; 
  }
  disk_write(filesys_disk, disk_inode->doubly, buf);
  return true;

}


/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode) 
{
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode) 
{
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  return inode->data.length;
}


struct cache_entry *
is_hit(disk_sector_t sector_idx){
  struct list_elem *e;
  struct cache_entry *c;
  for(e = list_begin(&cache_list); e!= list_end(&cache_list); e = list_next(e)){
    c = list_entry (e, struct cache_entry, elem);
    if(c->sector_idx == sector_idx){
      return c;
    }
  }
  return NULL;

}

struct cache_entry *
new_entry(void){
  struct cache_entry * c;
  if(list_size(&cache_list) >= 64){
    c = list_entry(list_pop_front(&cache_list), struct cache_entry, elem);
    disk_write(filesys_disk, c->sector_idx, c->data);
    //ASSERT(0);
  }
  else{
    c = malloc(sizeof *c);
  }

  return c;

}

int
min(int a, int b){
  if(a < b)
    return a;
  else
    return b;
}
<<<<<<< HEAD





bool
direct_allocate(int start_sector, int cnt, struct inode_disk * disk_inode)
{
  int i;
  for (i = 0; i < cnt; i++){
    if(disk_inode->direct[i+start_sector-1] == 0)
    {
      if (!free_map_allocate (1, &disk_inode->direct[i+start_sector-1])){
        ASSERT(0);
        return false;
      }
      static char zeros[DISK_SECTOR_SIZE];
      disk_write (filesys_disk, disk_inode->direct[i+start_sector-1], zeros); 
    }
  }

  return true;
}




bool
indirect_allocate(int start_sector, int cnt, disk_sector_t indirect)
{
  disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
  //memset(&buf, 0, DISK_SECTOR_SIZE);

  
  disk_read(filesys_disk, indirect, buf);

  int i;
  for(i = 0; i < cnt; i++)
  {
    if (!free_map_allocate (1, &buf[i+start_sector-1])){
      return false;
    }
    static char zeros[DISK_SECTOR_SIZE];



    disk_write (filesys_disk, buf[i+start_sector-1], zeros); 

  }
  disk_write(filesys_disk, indirect, buf);

/*
  if(last_sector > INDIRECT_CNT){
    indirect_allocate(1, last_sector-INDIRECT_CNT, disk_inode->doubly);
  }
*/
  return true;



}

bool
doubly_allocate(int start_sector, int cnt, disk_sector_t doubly)
{
  

  disk_sector_t buf[DISK_SECTOR_SIZE/sizeof(disk_sector_t)];
  int i = 0;
  int index;

  disk_read(filesys_disk, doubly, buf);

  if(start_sector % INDIRECT_CNT != 1)
  {
    indirect_allocate(start_sector % INDIRECT_CNT, min(cnt, INDIRECT_CNT-(start_sector%INDIRECT_CNT)), buf[start_sector/INDIRECT_CNT]);
    cnt -= min(cnt, INDIRECT_CNT-(start_sector%INDIRECT_CNT));
    index = start_sector/INDIRECT_CNT + 1;
  }




  while(cnt > 0){
    if (!free_map_allocate (1, &buf[index]))
      return false;
    indirect_allocate(1, min(cnt, INDIRECT_CNT), buf[index]);
    cnt -= INDIRECT_CNT;
    index++;
  }

  disk_write(filesys_disk, doubly, buf);
  return true;

}
=======
>>>>>>> 8bfb090a4d7031add0f02cb7cc6a27caecdbb7f3
