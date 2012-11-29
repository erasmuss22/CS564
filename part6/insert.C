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

	Status status;
	AttrDesc* attrs = new AttrDesc[attrCnt];
	AttrDesc* rel = new AttrDesc[attrCnt];
	Record rec;
	int tmpCnt;
	
	
	// Get the information about the relation
	if ((status = attrCat->getRelInfo(relation, tmpCnt, rel)) != OK){
			delete[] attrs;
			delete[] rel;
			return status;
	}
	
	
	// Check if the number attributes supplied matches the number of attributes
	// in the relation
	if (attrCnt != tmpCnt){
		delete[] attrs;
		delete[] rel;
		return ATTRTYPEMISMATCH;
	}
	
	
	// Calculate the total length of the attributes to be inserted
	int width = 0;
	for (int i = 0; i < tmpCnt; i++){
		width += rel[i].attrLen;
	}

	
	// Set up a record to have data copied into at the correct offsets
	char* outputData = new char[width];
	rec.data = (void *) outputData;
	rec.length = width;
	
	
	// Get all the AttrDesc for each attrInfo supplied in the parameters
	for (int i = 0; i < attrCnt; i++){
		
			if ((status = attrCat->getInfo(relation, attrList[i].attrName, attrs[i])) != OK){
				delete attrs;
				delete rel;
				return status;
			}
			
	}
	
	
	// For each attribute, copy its data into the record by using
	// the correct offset. First check the attribute type to get
	// the proper copying of data into the record.
	for (int i = 0; i < attrCnt; i++){
			
		int tempi;
		float tempf;
		
		
			switch (attrs[i].attrType) {
					
					// attribute is integer, so use atoi to convert
					// its value to an int
					case INTEGER:
					
						tempi = atoi((char*) attrList[i].attrValue);
						memcpy(outputData + attrs[i].attrOffset, &tempi,
							   attrs[i].attrLen);
							   
						break;
						
					// attribute is a float, so use atof to convert
					// its value to a float	
					case FLOAT:
					
						tempf = atof((char*) attrList[i].attrValue);
						memcpy(outputData + attrs[i].attrOffset, &tempf,
							   attrs[i].attrLen);
							   
						break;
						
					case STRING:
					
						memcpy(outputData + attrs[i].attrOffset, attrList[i].attrValue,
							   attrs[i].attrLen);
							   
						break;
			}
	}
		
	
	
	// Open an InsertFileScan on the relation that the records
	// must be inserted into
	
	InsertFileScan* ifs = new InsertFileScan(relation, status);
	if (status != OK){
		delete[] outputData;
		delete ifs;
		delete[] attrs;
		delete[] rel;
		return status;
	}
	
	
	RID rid;
	status = ifs->insertRecord(rec, rid);
	
	// Final cleanup if there were no problems
	delete[] outputData;
	delete ifs;
	delete[] attrs;
	delete[] rel;
	
	return status;

}

