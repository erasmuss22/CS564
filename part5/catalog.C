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
  
  scan->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  
  while( (status = scan->scanNext(rid)) != FILEEOF ) {
	if (status != OK) return status;
	
	if ((status = scan->getRecord(rec)) != OK) return status;
	
	memcpy(&record, rec.data, sizeof(RelDesc));
	delete scan;
	return OK;
  }
  
  delete scan;
  
  return RELNOTFOUND;

}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan* ifs;
  Status status;
  Record rec;
  
  ifs = new InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  rec.length = sizeof(RelDesc);
  rec.data = &record;
  
  
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

  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) == OK)
        {

            if ((status = hfs->getRecord(rec)) != OK) break;
			memcpy(&temp, rec.data, sizeof(AttrDesc));
			if (temp.attrName == attrName){
				memcpy(&record, rec.data, sizeof(AttrDesc));
				delete hfs;
				return OK;
			}
            
        }

		// Didn't find a record
		//hfs->endScan();
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
  RelDesc rd;
  
  
  rec.length = sizeof(AttrDesc);
  rec.data = &record;
  
  
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
  
  //Maybe want to return status here?
  if (status != OK) error.print(status);

  
  
  //attrCat->startScan(sizeof(record.relName), sizeof(record.attrName), STRING, relation, EQ);
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) != FILEEOF)
  {
	// reconstruct record i
     if ((status = hfs->getRecord(rec)) != OK) break;
    memcpy(&temp, rec.data, sizeof(AttrDesc));
    if (temp.attrName == attrName){
	  // Delete the record
	  if ((status = hfs->deleteRecord()) != OK) return status;
				
	  //hfs->endScan();
	  delete hfs;
	  return OK;
    }
  }


		// Didn't find a record
		//hfs->endScan();
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
 
   if ((status = relCat->getInfo(relation, rd)) != OK) return status;
   attrCnt = rd.attrCnt;
   attrs = new AttrDesc[attrCnt];
	
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) error.print(status);
  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);

  int count = 0;
  while ((status = hfs->scanNext(rid)) == OK)
        {

            if ((status = hfs->getRecord(rec)) != OK) break;
			memcpy(&attrs[count], rec.data, sizeof(AttrDesc));
			
            count++;

        }

	
	// Didn't find a record
		//hfs->endScan();
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

