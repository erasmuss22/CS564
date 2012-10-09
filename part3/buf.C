#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++) 
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}


BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) 
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}


const Status BufMgr::allocBuf(int & frame) 
{
advanceClock();
	unsigned int startPos = clockHand;
	unsigned int currPos = clockHand;
	BufDesc* currDesc;
	
	// Iterate through the frames looking for a useable one.
	// This terminates after reaching the starting clockHand
	// position again.
	do 
	{
		currDesc = &bufTable[currPos];
		
		// If the current frame is valid continue to check other
		// informational variables.
		if( currDesc.valid ) {
			//  If the refbit is true it is unusable. Advance the clock
			if( currDesc.refbit ) {
				currDesc.refbit = false;
				advaceClock();
				currPos = clockHand;
			}
			// If the pin count is > 0 it is also unuseable because other processes,
			// are using it.  Advance the clock.
			else if (currDesc.pinCnt > 0) {
				advanceClock();
				currPos = clockHand;
			}
			// Found a useable frame
			else {
				// Check to see if we need to write the page to file before using.
				if (currDesc.dirty) {
					status = currDesc.file->writePage(currDesc.pageNo, &(bufPool[currPos]));
					if (status != OK)
						return status;
				}
				
				// Frame was valid so we need to remove it from the hashTable before allocating.
				frame = clockHand;
				status = hashTable->remove(currDesc.file, currDesc.pageNo);
				return status;
			}
		}
		// Frame was invalid so we can use it.  Checking to see if the page is dirty, but still need
		// to work through this to see if it's necessary.
		
		else {
			if (currDesc.dirty) {
				status = currDesc.file->writePage(currDesc.pageNo, bufPool[currPos]);
				if (status != OK)
						return status;
			}
			frame = clockHand;
			return OK;
		}
	
	} while (currPos != startPos);
	
	return BUFFEREXCEEDED;
	
}
	


	
const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{
	Status status;
	int frame;
	
	// Check if the pageNo of the file is in the hashtable
	if ((status = hashTable->lookup(file, PageNo, frame)) == OK){
		// If it is, set the refpulbit to true and increment pinCnt
		bufTable[frame].refbit = true;
		bufTable[frame].pinCnt++;
		return OK;
	} else {
		/* If it isn't in the hashtable, find the next frame it could be
		in and read the page from the disk into that frame. Then set
		up the page in the buffer and insert it into the hashtable */
		if((status = allocBuf(frame)) != OK) return status;
		if((status = file->readPage(PageNo, &(bufPool[frame]))) != OK) return status;
		if((status = hastTable->insert(file, PageNo, frame)) != OK) return status;
		bufTable[frame].Set(file, PageNo);
		return status;
	}
}


const Status BufMgr::unPinPage(File* file, const int PageNo, 
			       const bool dirty) 
{
	Status status;
	int frame
	// Lookup pageNo of file to see if it's in the hashtable
	if ((status = hashTable->lookup(file, PageNo, frame)) == OK){
		// Check if the pinCnt is at 0
		if (bufTable[frame].pinCnt == 0){
			if (dirty == true){
				bufTable[frame].dirty = true;
			}
			return PAGENOTPINNED
		} else{
			if (dirty == true){
				// Set the dirty bit
				bufTable[frame].dirty = true;
			} 
			// Decrement the pinCnt
			bufTable[frame].pinCnt--;
			return OK;
		}
	} else {
		return status;
	}



}

const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page) 
{
	Status status;
	int frame;
	// Allocate the page in the file and set the pageNo
	if((status = file->allocatePage(pageNo)) != OK) return status;
	// Read the page into the buffer pool
	if((status = readPage(file, pageNo, page)) != OK) return status;
	return status;
}

const Status BufMgr::disposePage(File* file, const int pageNo) 
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file) 
{
  Status status;

  for (int i = 0; i < numBufs; i++) {
    BufDesc* tmpbuf = &(bufTable[i]);
    if (tmpbuf->valid == true && tmpbuf->file == file) {

      if (tmpbuf->pinCnt > 0)
	  return PAGEPINNED;

      if (tmpbuf->dirty == true) {
#ifdef DEBUGBUF
	cout << "flushing page " << tmpbuf->pageNo
             << " from frame " << i << endl;
#endif
	if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
	  return status;

	tmpbuf->dirty = false;
      }

      hashTable->remove(file,tmpbuf->pageNo);

      tmpbuf->file = NULL;
      tmpbuf->pageNo = -1;
      tmpbuf->valid = false;
    }

    else if (tmpbuf->valid == false && tmpbuf->file == file)
      return BADBUFFER;
  }
  
  return OK;
}


void BufMgr::printSelf(void) 
{
    BufDesc* tmpbuf;
  
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i]) 
             << "\tpinCnt: " << tmpbuf->pinCnt;
    
        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


