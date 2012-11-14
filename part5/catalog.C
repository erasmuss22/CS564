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
  
  scan->startScan(0, (int)(sizeof record.relName), STRING, relation.c_str(), EQ);
  
  while( (status = scan->scanNext(rid)) != FILEEOF ) {
	if (status != OK) return status;
	
	if ((status = scan->getRecord(rec)) != OK) return status;
	memcpy(&record, rec.data, sizeof rec.data);
  }
  
  if (status != FILEEOF && status != OK) return status;
  else return OK;

}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan* ifs;
  Status status;
  Record rec;
  
  ifs = new InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  rec.data = &record;
  rec.length = sizeof(RelDesc);
  
  status = ifs->insertRecord(rec, rid);
  
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
  
  hfs->startScan(0, (int)(sizeof rd.relName), STRING, relation.c_str(), EQ);
  
  while( (status = hfs->scanNext(rid)) != FILEEOF ) {
	if (status != OK) return status;
	
    if ( (status = hfs->deleteRecord()) != OK) return status;
  }
  
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
			memcpy(&temp, rec.data, sizeof rec.data);
			if (temp.attrName == attrName){
				memcpy(&record, &rec.data, sizeof rec.data);
				return OK;
			}
            
        }

		// Didn't find a record
		hfs->endScan();
		if(status == FILEEOF) {
			return ATTRNOTFOUND;
		} else {
			return status;
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
  memcpy(rec.data, &record, rec.length);
  
  ifs = new InsertFileScan(ATTRCATNAME, status);
  if ((status = ifs->insertRecord(rec, rid)) != OK) return status;
  
  // Must add the attribute to relCat	
  // Start by getting the record to update
  if ((status = relCat->getInfo(record.relName, rd)) != OK) return status;

  // Remove the record from the relCat table
  if ((status = relCat->removeInfo(record.relName)) != OK) return status;
				
  // Increase the attribute count because we are adding 1 attribute
  rd.attrCnt += 1;
				
  // Add back to the table -- by deleting and then adding we
  // have updated the record
  if ((status = relCat->addInfo(rd)) != OK) return status;
  
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
  char an[MAXNAME];
  RelDesc rd;

  if (relation.empty() || attrName.empty()) return BADCATPARM;
		
  hfs = new HeapFileScan(ATTRCATNAME, status);
  
  //Maybe want to return status here?
  if (status != OK) error.print(status);

  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) == OK)
        {

            if ((status = hfs->getRecord(rec)) != OK) break;
			// Get the attribute name by taking correct offset from
			// the start of the record (like in dbcreate.C)
			memcpy(an, (char *) rec.data+MAXNAME, MAXNAME);

			if (strcmp(an, attrName.c_str()) == 0){
				
			}
            
        }
  
  //attrCat->startScan(sizeof(record.relName), sizeof(record.attrName), STRING, relation, EQ);
  hfs->startScan(0, sizeof record.relName, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) != FILEEOF)
  {
    // reconstruct record i
     if ((status = hfs->getRecord(rec)) != OK) break;
    memcpy(&temp, &rec.data, sizeof rec.data);
    if (temp.attrName == attrName){
	  status = hfs->deleteRecord();
	  // Delete the record
	  if ((status = hfs->deleteRecord()) != OK) return status;
				
	  // Must remove the attribute from relCat	
	  // Start by getting the record to update
	  if ((status = relCat->getInfo(relation, rd)) != OK) return status;

	  // Remove the record from the relCat table
	  if ((status = relCat->removeInfo(relation)) != OK) return status;
				
	  // Decrease the attribute count because we are deleting 1 attribute
	  rd.attrCnt -= 1;
				
	  // Add back to the table -- by deleting and then adding we
	  // have updated the record
	  if ((status = relCat->addInfo(rd)) != OK) return status;

	  hfs->endScan();
	  return OK;
    }
  }


		// Didn't find a record
		hfs->endScan();
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
  HeapFileScan*  hfs, *relScan;
  RelDesc rd;
  int numAttrDesc = 0;

  if (relation.empty()) return BADCATPARM;

  relScan = new HeapFileScan(RELCATNAME, status);
  if (status != OK) error.print(status);
  
  relScan->startScan(0, (int)(sizeof rd.relName), STRING, relation.c_str(), EQ);
  while ((status = relScan->scanNext(rid)) != FILEEOF) {
	if (status != OK) break;
	if ( (status = relScan->getRecord(rec)) != OK) break;
	
	memcpy(&rd, rec.data, sizeof rec.data);
	numAttrDesc += rd.attrCnt;
   }
   
   if (status != FILEEOF && status != OK) return status;
   
   attrs = new AttrDesc[numAttrDesc];
	
	
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) error.print(status);

  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);


  int count = 0;
  while ((status = hfs->scanNext(rid)) == OK)
        {

            if ((status = hfs->getRecord(rec)) != OK) break;
			memcpy(&attrs[count], &rec.data, sizeof rec.data);
            count++;

        }

	
	// Didn't find a record
		hfs->endScan();
		if(status == FILEEOF) {
			return ATTRNOTFOUND;
		} else {
			return status;
		}
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

