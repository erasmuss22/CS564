#include "heapfile.h"
#include "error.h"

// routine to create a heapfile
const Status createHeapFile(const string fileName)
{
    File* 		file;
    Status 		status;
    FileHdrPage*	hdrPage;
    int			hdrPageNo;
    int			newPageNo;
    Page*		newPage;

    // try to open the file. This should return an error
    status = db.openFile(fileName, file);
    if (status != OK)
    {
		// file doesn't exist. First create it and allocate
		// an empty header page and data page. Also must open it
		if ((status = db.createFile(fileName)) != OK) return status;
		if ((status = db.openFile(fileName, file)) != OK) return status;
		
		
		// Allocate the page which sets a pointer for newPage
		if ((status = bufMgr->allocPage(file, hdrPageNo, newPage)) != OK) return status;
		
		
		// Cast Page to FileHdrPage and initialize hdrPage
		hdrPage = (FileHdrPage *) newPage;
		
		
		// Set first page of file by calling allocPage
		if ((status = bufMgr->allocPage(file, newPageNo, newPage)) != OK) return status;
		
		
		// Initialize the first page which also sets its next page to -1
		newPage->init(newPageNo);
		
		
		// Set the first and last page variables
		hdrPage->firstPage = newPageNo;
		hdrPage->lastPage = newPageNo;
		hdrPage->pageCnt = 1;
		hdrPage->recCnt = 0;
		
		
		// Unpin the pages and set their dirty bits
		if ((status = bufMgr->unPinPage(file, newPageNo, true)) != OK) return status;
		if ((status = bufMgr->unPinPage(file, hdrPageNo, true)) != OK) return status;
		
		
		// Flush the file to disk and close it
		if ((status = bufMgr->flushFile(file)) != OK) return status;
		if ((status = db.closeFile(file)) != OK) return status;
		return OK;	
		
    }
    return (FILEEXISTS);
}

// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
	return (db.destroyFile (fileName));
}

// constructor opens the underlying file
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status 	status;
    Page*	pagePtr;

    cout << "opening file " << fileName << endl;

    // open the file and read in the header page and the first data page
    if ((status = db.openFile(fileName, filePtr)) == OK)
    {
		// start by getting the header page of the file
		if ((status = filePtr->getFirstPage(headerPageNo)) != OK) {
			returnStatus = status;
			return;
		}
		
		
		// Read and pin the header page
		if ((status = bufMgr->readPage(filePtr, headerPageNo, pagePtr)) != OK){
			returnStatus = status;
			return;
		} 
		
		
		
		// Initialize class variables
		hdrDirtyFlag = false;
		headerPage = (FileHdrPage *) pagePtr;
		
		
		
		// Read and pin the first page while initializing cur variables
		curPageNo = headerPage->firstPage;
		if ((status = bufMgr->readPage(filePtr, curPageNo, curPage)) != OK){
			returnStatus = status;
			return;
		}
		curDirtyFlag = false;
		curRec = NULLRID;
		returnStatus = status = OK;
		return;
		
    }
    else
    {
    	cerr << "open of heap file failed\n";
		returnStatus = status;
		return;
    }
}

// the destructor closes the file
HeapFile::~HeapFile()
{
    Status status;
    cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

    // see if there is a pinned data page. If so, unpin it 
    if (curPage != NULL)
    {
    	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		curPage = NULL;
		curPageNo = 0;
		curDirtyFlag = false;
		if (status != OK) cerr << "error in unpin of date page\n";
    }
	
	 // unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";
	
	// status = bufMgr->flushFile(filePtr);  // make sure all pages of the file are flushed to disk
	// if (status != OK) cerr << "error in flushFile call\n";
	// before close the file
	status = db.closeFile(filePtr);
    if (status != OK)
    {
		cerr << "error in closefile call\n";
		Error e;
		e.print (status);
    }
}

// Return number of records in heap file

const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}

// retrieve an arbitrary record from a file.
// if record is not on the currently pinned page, the current page
// is unpinned and the required page is read into the buffer pool
// and pinned.  returns a pointer to the record via the rec parameter

const Status HeapFile::getRecord(const RID & rid, Record & rec)
{
    Status status;

    cout<< "getRecord. record (" << rid.pageNo << "." << rid.slotNo << ")" << endl;
    
	// Check to see if the desired record is currently pinned
	if (rid.pageNo == curPageNo){
		// It is pinned so simply get the record and do book keeping
		status = curPage->getRecord(rid, rec);
		curRec = rid;
		return status;
	}
    
	
	// The desired page isn't pinned, so we must unpin it
	if ((status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag)) != OK) return status;
	
	// Read the desired page into the buffer pool, get record, and do book keeping
	if ((status = bufMgr->readPage(filePtr, rid.pageNo, curPage)) != OK) return status;
	status = curPage->getRecord(rid, rec);
    
	curRec = rid;
	curPageNo = rid.pageNo;
	curDirtyFlag = false;
	
	return status;

   
}

HeapFileScan::HeapFileScan(const string & name,
			   Status & status) : HeapFile(name, status)
{
    filter = NULL;
}

const Status HeapFileScan::startScan(const int offset_,
				     const int length_,
				     const Datatype type_, 
				     const char* filter_,
				     const Operator op_)
{
    if (!filter_) {                        // no filtering requested
        filter = NULL;
        return OK;
    }
    
    if ((offset_ < 0 || length_ < 1) ||
        (type_ != STRING && type_ != INTEGER && type_ != FLOAT) ||
        (type_ == INTEGER && length_ != sizeof(int)
         || type_ == FLOAT && length_ != sizeof(float)) ||
        (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ != NE))
    {
        return BADSCANPARM;
    }

    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;
}


const Status HeapFileScan::endScan()
{
    Status status;
    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
		curDirtyFlag = false;
        return status;
    }
    return OK;
}

HeapFileScan::~HeapFileScan()
{
    endScan();
}

const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}

const Status HeapFileScan::resetScan()
{
    Status status;
    if (markedPageNo != curPageNo) 
    {
		if (curPage != NULL)
		{
			status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
			if (status != OK) return status;
		}
		// restore curPageNo and curRec values
		curPageNo = markedPageNo;
		curRec = markedRec;
		// then read the page
		status = bufMgr->readPage(filePtr, curPageNo, curPage);
		if (status != OK) return status;
		curDirtyFlag = false; // it will be clean
    }
    else curRec = markedRec;
    return OK;
}


const Status HeapFileScan::scanNext(RID& outRid)
{
    
	Status 	status = OK;
    RID		nextRid;
    RID		tmpRid;
    int 	nextPageNo;
    Record      rec;
	bool    matched = false;
	bool    validRecord = false;
		if (curPageNo == -1) return FILEEOF;
		
		
		// get the very first page, looking at the constructor, it looks like
		// curPage is set, so it will never be NULL
		/*if (curPage == NULL){
			//cout << "1" << endl;
			curPageNo = headerPage->firstPage;
			if (curPageNo == -1) return FILEEOF;
			if ((status = bufMgr->readPage(filePtr, curPageNo, curPage)) != OK) return status;
			if ((status = curPage->getNextPage(nextPageNo)) != OK) return status;
			if ((status = curPage->firstRecord(curRec)) != OK) {
				if ((status = bufMgr->unPinPage(filePtr, curPageNo, false)) != OK) return status;
				curPageNo = -1;
				return FILEEOF;
			}
			if ((status = curPage->getRecord(curRec, rec)) != OK) return status;
			if (matchRec(rec) == true){
				outRid = curRec;
				return OK;
			}
			
		} */

    do{

			validRecord = false;
			
			// Attempt to get the next record. If it fails, must look towards
			// next page
			if ((status = curPage->nextRecord(curRec, nextRid)) != OK) {
				    
					// Get the next page
					if ((status = curPage->getNextPage(nextPageNo)) != OK) return status;
					
					
					// Check to see if the next page exists
					if (nextPageNo != -1) {
						
						// Page does exist, so we unpin the current one and read in the next
						if ((status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag)) != OK)return status;
						if ((status = bufMgr->readPage(filePtr, nextPageNo, curPage)) != OK) return status;
						curPageNo = nextPageNo;

						// Get the first record on the new page. If no error is returned, this
						// is a valid record and we can check it for a match
						if ((status = curPage->firstRecord(curRec)) == OK) validRecord = true;
					}   
					
					 // The page doesn't exist, so we return FILEEOF
					 else {
						return FILEEOF;
					} 
			} 
			
			 // Getting the nextRecord didn't have an error, so we set curRec since this
			 // is a valid record
			 else {
				curRec = nextRid;
				validRecord = true;
			}
			
			// If a record is valid, we can get the pointer to the actual record and
			// check for a match
			if (validRecord == true){
				
				if ((status = curPage->getRecord(curRec, rec)) != OK) return status;
				matched = matchRec(rec);
				
				if (matched == true) {

					outRid = curRec;
					return OK;
					
				}
			}
	} while (!matched);
	

}


// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page 

const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}

// delete record from file. 
const Status HeapFileScan::deleteRecord()
{
    Status status;

    // delete the "current" record from the page
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true; 
    return status;
}


// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}

const bool HeapFileScan::matchRec(const Record & rec) const
{
    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

    float diff = 0;                       // < 0 if attr < fltr
    switch(type) {

    case INTEGER:
        int iattr, ifltr;                 // word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;               // word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;
}

InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will bread the header page and the first
  // data page of the file into the buffer pool
}

InsertFileScan::~InsertFileScan()
{
    Status status;
    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }
}

// Insert a record into the file
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page*	newPage;
    int		newPageNo;
    Status	status, unpinstatus;
    RID		rid;


    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }

  
    // Since this is a heap file, start by setting the current page to
	// being the last page of the file. Must check if curPage is null,
	// otherwise it has a pinCnt of like 14
	if (curPage == NULL){
		curPageNo = headerPage->lastPage;
		
		
		// Then read it into the buffer pool
		if ((status = bufMgr->readPage(filePtr, curPageNo, curPage)) != OK) return status;
	}
	
	// Add the desired record to the last page, if this doesn't work
	// that page is full and we must add another page to the end and do
	// the respective book keeping
	if ((status = curPage->insertRecord(rec, rid)) == OK){
		outRid = rid;		   // return rid of inserted record	
		hdrDirtyFlag = true;  // Inserted record, thus page is now dirty
		curDirtyFlag = true;
		headerPage->recCnt += 1; // update the record count in header page
		return OK;
	} 
	
	// Inserting the record didn't work b/c the page was full
	
	else {

		// Start by allocating a new page via the bufMgr and initializing the page
		if ((status = bufMgr->allocPage(filePtr, newPageNo, newPage)) != OK) return status;
		newPage->init(newPageNo);
		
		// Since this will be the new last page, set its next page to point to
		// -1. Since it only returns OK, no need to check the status. We also
		// update the header page meta data because it points to a new last page
		status = newPage->setNextPage(-1);
		headerPage->lastPage = newPageNo;
		headerPage->pageCnt += 1; // Added a page, update the page count
		hdrDirtyFlag = true;
		
		// In addition to updating the headerPage meta data, we must set the
		// current page (the one that is full) to point to the new page to
		// keep the heap file as a linked list
		status = curPage->setNextPage(newPageNo);
		
		// We are no longer using the current page, so unpin it
		if ((status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag)) != OK){
			// extra things for when  the page is already unpinned? unpinstatus
			return status;
		} 

		// Set the current page to the new one and then try to insert the record
		curPageNo = newPageNo;
		curPage = newPage;
		if ((status = curPage->insertRecord(rec, rid)) == OK){
			outRid = rid;		   // return rid of inserted record	
			hdrDirtyFlag = true;  // Inserted record, thus page is now dirty
			curDirtyFlag = true;
			headerPage->recCnt += 1; // update the record count in header page
			return OK;
		}
		return status;
		
	}
  
  
  
  
  
  
  
  
  
}


