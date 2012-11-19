#include "catalog.h"

RelCatalog::RelCatalog(Status &status) :
	 HeapFile(RELCATNAME, status)
{
// nothing should be needed here
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
  if (relation.empty())
    return BADCATPARM;
  Status status;
  Record rec;
  RID rid;
  
  //relCat is not of the HeapFileScan type, so create a scan object
  HeapFileScan* scan = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  
  // Scan for all tuples with the relation name passed into the
  // function
  scan->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  
  
  // Search until a record is found and copy it into the record
  // parameter for return
  while( (status = scan->scanNext(rid)) != FILEEOF ) {
	if (status != OK) return status;
	
	if ((status = scan->getRecord(rec)) != OK) return status;
	
	memcpy(&record, rec.data, sizeof(RelDesc));
	delete scan;
	return OK;
  }
  
  // EOF reached, so relation wasn't found
  delete scan;
  return RELNOTFOUND;

}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan* ifs;
  Status status;
  Record rec;
  
  // Start an InsertFileScan
  ifs = new InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  
  // Copy the record data into an actual record
  rec.length = sizeof(RelDesc);
  rec.data = &record;
  
  
  // Insert the record
  status = ifs->insertRecord(rec, rid);
  
  delete ifs;
  return status;

}

const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;
  RelDesc rd;
  if (relation.empty()) return BADCATPARM;
  
  
  hfs = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  
  // find the record with the given relation name and delete it
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while( (status = hfs->scanNext(rid)) != FILEEOF ) {
	
	if (status != OK) return status;
    if ( (status = hfs->deleteRecord()) != OK) return status;
	
  }
  
  delete hfs;
  
  if (status != FILEEOF && status != OK) return status;
  else return OK;


}


RelCatalog::~RelCatalog()
{
// nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
	 HeapFile(ATTRCATNAME, status)
{
// nothing should be needed here
}


const Status AttrCatalog::getInfo(const string & relation, 
				  const string & attrName,
				  AttrDesc &record)
{

  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;
  AttrDesc temp;

  if (relation.empty() || attrName.empty()) return BADCATPARM;

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) error.print(status);


  // Scan for all attrCat tuples that match the relation string
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) == OK)
        {

			// get the record and copy its data
            if ((status = hfs->getRecord(rec)) != OK) break;
			memcpy(&temp, rec.data, sizeof(AttrDesc));
			
			
			// if the attrName of the record is the same as the input
			// parameter, copy it into the return value and return
			if (temp.attrName == attrName){
				memcpy(&record, rec.data, sizeof(AttrDesc));
				delete hfs;
				return OK;
			}
            
        }

		// Didn't find a record
		delete hfs;
		if(status == FILEEOF) {
			return ATTRNOTFOUND;
		} else {
			return OK;
		}

}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  InsertFileScan* ifs;
  Status status;
  Record rec;
  
  
  // copy the record data into a record
  rec.length = sizeof(AttrDesc);
  rec.data = &record;
  
  
  // insert the record. All bookkeeping is done by the calling method,
  // so no need to do it here
  ifs = new InsertFileScan(ATTRCATNAME, status);
  if ((status = ifs->insertRecord(rec, rid)) != OK) return status;
  
  
  delete ifs;
  
  return status;
  


}


const Status AttrCatalog::removeInfo(const string & relation, 
			       const string & attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record, temp;
  HeapFileScan*  hfs;
  RelDesc rd;
  
  if (relation.empty() || attrName.empty()) return BADCATPARM;	
  hfs = new HeapFileScan(ATTRCATNAME, status);
  

  if (status != OK) error.print(status);

  
  
  // start a scan searching for the relation name
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) != FILEEOF)
  {

	  // get and copy the record data into an AttrDesc
     if ((status = hfs->getRecord(rec)) != OK) break;
     memcpy(&temp, rec.data, sizeof(AttrDesc));
	 
	 
	 // if the attribute name is the same as the input param,
	 // delete the record
     if (temp.attrName == attrName){
		 
	  if ((status = hfs->deleteRecord()) != OK) return status;
				
	  delete hfs;
	  return OK;
    }
  }


	// Didn't find a record
	 delete hfs;
	 if(status == FILEEOF) {
		return ATTRNOTFOUND;
	 } else {
	 	return status;
	 }

}


const Status AttrCatalog::getRelInfo(const string & relation, 
				     int &attrCnt,
				     AttrDesc *&attrs)
{
  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;
  RelDesc rd;
  int numAttrDesc = 0;

  if (relation.empty()) return BADCATPARM;
 
  // check if the relation exists. If it does, get its attrCnt and
  // initialize the array for storing all tuples
  if ((status = relCat->getInfo(relation, rd)) != OK) return status;
  attrCnt = rd.attrCnt;
  attrs = new AttrDesc[attrCnt];
	
	
  // start a scan searching for all tuples of the relation name	
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) error.print(status);
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);

  
  // get every record and store it in the array
  int count = 0;
  while ((status = hfs->scanNext(rid)) == OK) {

            if ((status = hfs->getRecord(rec)) != OK) break;
			memcpy(&attrs[count], rec.data, sizeof(AttrDesc));
            count++;

  }

	
	// Didn't find a record
	delete hfs;
	if(status == FILEEOF) {
		return OK;
	} else {
		return OK;
	}
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

