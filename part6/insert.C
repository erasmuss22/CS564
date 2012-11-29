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
	int tmpCnt;
	
	if ((status = relCat->getInfo(relation, rd)) != OK){
			delete attrs;
			delete rel;
			return status;
		}
		
	if ((status = attrCat->getRelInfo(relation, tmpCnt, rel)) != OK){
			delete attrs;
			delete rel;
			return status;
	}
	
	if (attrCnt != tmpCnt){
		delete[] attrs;
		delete[] rel;
		return ATTRTYPEMISMATCH;
	}
	
	int width = 0;
	for (int i = 0; i < tmpCnt; i++){
		width += rel[i].attrLen;
	}

	char* outputData = new char[width];
	rec.data = (void *) outputData;
	rec.length = width;
	
	for (int i = 0; i < attrCnt; i++){
			if ((status = attrCat->getInfo(relation, attrList[i].attrName, attrs[i])) != OK){
				delete attrs;
				delete rel;
				return status;
			}
	}
	
	//for (int j = 0; j < tmpCnt; j++){
		for (int i = 0; i < attrCnt; i++){
			/*if ((status = attrCat->getInfo(relation, attrList[i].attrName, attrs[i])) != OK){
				delete attrs;
				delete rel;
				return status;
			}*/
			
			int tempi;
			float tempf;
		//	if (string(rel[j].attrName) == string(attrs[i].attrName)){
				switch (attrs[i].attrType) {
						case INTEGER:
							tempi = atoi((char*) attrList[i].attrValue);
							memcpy(outputData + attrs[i].attrOffset,
								   &tempi,
								   attrs[i].attrLen);
							break;
						case FLOAT:
							tempf = atof((char*) attrList[i].attrValue);
							memcpy(outputData + attrs[i].attrOffset,
								   &tempf,
								   attrs[i].attrLen);
							break;
						case STRING:
							memcpy(outputData + attrs[i].attrOffset,
								   attrList[i].attrValue,
								   attrs[i].attrLen);
							break;
						}
			//}
		}
		
	//}
	
	
	
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
	
	delete[] outputData;
	delete ifs;
	delete[] attrs;
	delete[] rel;
	
	return OK;

}

