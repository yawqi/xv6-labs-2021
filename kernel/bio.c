// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"

#include "param.h"

#include "spinlock.h"

#include "sleeplock.h"

#include "riscv.h"

#include "defs.h"

#include "fs.h"

#include "buf.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct spinlock bucket_locks[NBUCKETS];
  struct buf buckets[NBUCKETS];
} bcache;

void binit(void) {
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  for (int bkt = 0; bkt < NBUCKETS; bkt++) {
    initlock(&bcache.bucket_locks[bkt], "bcache_bkt");
    bcache.buckets[bkt].prev = &bcache.buckets[bkt];
    bcache.buckets[bkt].next = &bcache.buckets[bkt];
  }

  // Create linked list of buffers
  for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
    initsleeplock(&b->lock, "buffer");
    b->next = bcache.buckets[0].next;
    b->prev = &bcache.buckets[0];
    bcache.buckets[0].next->prev = b;
    bcache.buckets[0].next = b;
  }
}

static uint get_hash(uint blockno) { return blockno % NBUCKETS; }

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf *bget(uint dev, uint blockno) {
  struct buf *b;
  uint bkt = get_hash(blockno);

  acquire(&bcache.bucket_locks[bkt]);
  // Is the block already cached?
  for (b = bcache.buckets[bkt].next; b != &bcache.buckets[bkt]; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      release(&bcache.bucket_locks[bkt]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.

  // struct buf *victim;
  // uint min_ticks = 0xffffffff;

  for (int bkt_idx = 0; bkt_idx < NBUCKETS; bkt_idx++) {
    if (bkt_idx == bkt)
      continue;
    acquire(&bcache.bucket_locks[bkt_idx]);
    for (b = bcache.buckets[bkt_idx].next; b != &bcache.buckets[bkt_idx];
         b = b->next) {
      if (b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
      }
      release(&bcache.bucket_locks[bkt_idx]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf *bread(uint dev, uint blockno) {
  struct buf *b;

  b = bget(dev, blockno);
  if (!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b) {
  if (!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void brelse(struct buf *b) {
  uint bkt;
  if (!holdingsleep(&b->lock))
    panic("brelse");

  bkt = get_hash(b->blockno);
  acquire(&bcache.bucket_locks[bkt]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->ticks = ticks;
  }

  release(&bcache.bucket_locks[bkt]);
  releasesleep(&b->lock);
}

void bpin(struct buf *b) {
  uint bkt = get_hash(b->blockno);
  acquire(&bcache.bucket_locks[bkt]);
  b->refcnt++;
  release(&bcache.bucket_locks[bkt]);
}

void bunpin(struct buf *b) {
  uint bkt = get_hash(b->blockno);
  acquire(&bcache.bucket_locks[bkt]);
  b->refcnt--;
  release(&bcache.bucket_locks[bkt]);
}
