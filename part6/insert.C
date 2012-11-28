#include "catalog.h"
#include "query.h"


/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation, 
	const int attrCnt, 
	const attrInfo attrList[])
{
// part 6
	Status status;
	AttrDesc* attrs = new AttrDesc[attrCnt];
	AttrDesc* rel = new AttrDesc[attrCnt];
	Record rec;
	RelDesc rd;
	
	if ((status = relCat->getInfo(relation, rd)) != OK){
			delete attrs;
			delete rel;
			return status;
		}
		
	if ((status = attrCat->getRelInfo(relation, rd.attrCnt, rel)) != OK){
			delete attrs;
			delete rel;
			return status;
	}
	
	int width = 0;
	for (int i = 0; i < rd.attrCnt; i++){
		width += rel[i].attrLen;
	}
	
	char outputData[width];
	rec.data = (void *) outputData;
	rec.length = width;
	
	
	for (int i = 0; i < attrCnt; i++){
		if ((status = attrCat->getInfo(relation, attrList[i].attrName, attrs[i])) != OK){
			delete attrs;
			delete rel;
			return status;
		}
		cout << attrs[i].attrLen << endl;
		cout << attrs[i].attrOffset << endl;
		cout << &(attrList[i].attrValue) << endl;
		cout << attrList[i].attrType << endl;
		memcpy(outputData + attrs[i].attrOffset,
                           attrList[i].attrValue,
                           attrs[i].attrLen);
	}
	
	
	
	InsertFileScan* ifs = new InsertFileScan(relation, status);
	if (status != OK){
		delete ifs;
		delete attrs;
		delete rel;
		return status;
	}
	RID rid;
	status = ifs->insertRecord(rec, rid);
	
	if (status != OK){
		delete ifs;
		delete attrs;
		delete rel;
		return status;
	}
	
	delete ifs;
	delete attrs;
	delete rel;
	
	return OK;

}

