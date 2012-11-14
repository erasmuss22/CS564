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
  HeapFileScan *scan = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  scan->startScan(0, (int)(sizeof record.relName), STRING, relation.c_str(), EQ);
  
  while( (status = scan->scanNext(rid)) != FILEEOF ) {
	if ((status = scan->getRecord(rec)) != OK) return status;
	memcpy(&record, rec.data, sizeof rec.data);
  }
  
  if (status != FILEEOF && status != OK) return status;
  else return OK;

}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan *ifs;
  Status status;
  Record rec;
  
  ifs = new InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  rec.data = &record;
  rec.length = sizeof(RelDesc);
  
  status = ifs->insertRecord(rec, rid));
  
  return status;

}

const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;



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
  char an[MAXNAME];
  

  if (relation.empty() || attrName.empty()) return BADCATPARM;

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) error.print(status);

  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) == OK)
        {

            if ((status = hfs->getRecord(rec)) != OK) break;
			// Get the attribute name by taking correct offset from
			// the start of the record (like in dbcreate.C)
			memcpy(an, (char *) rec.data+MAXNAME, MAXNAME);

			if (strcmp(an, attrName.c_str()) == 0){
				memcpy((char*)&record, rec.data, sizeof(AttrDesc));
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


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
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
  AttrDesc record;
  HeapFileScan*  hfs;
  char an[MAXNAME];
  RelDesc rd;

  if (relation.empty() || attrName.empty()) return BADCATPARM;
		
  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) error.print(status);

  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  while ((status = hfs->scanNext(rid)) == OK)
        {

            if ((status = hfs->getRecord(rec)) != OK) break;
			// Get the attribute name by taking correct offset from
			// the start of the record (like in dbcreate.C)
			memcpy(an, (char *) rec.data+MAXNAME, MAXNAME);

			if (strcmp(an, attrName.c_str()) == 0){
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
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) error.print(status);

  hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
  int count = 0;
  while ((status = hfs->scanNext(rid)) == OK)
        {

            if ((status = hfs->getRecord(rec)) != OK) break;
			// Get the attribute name by taking correct offset from
			// the start of the record (like in dbcreate.C)
			memcpy(an, (char *) rec.data+MAXNAME, MAXNAME);

			if (strcmp(an, attrName.c_str()) == 0){
				memcpy((char*)&record, rec.data, sizeof(AttrDesc));
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


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

