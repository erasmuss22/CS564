#include "catalog.h"
#include "query.h"
#include "stdio.h"
#include "stdlib.h"


// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
   // Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;
	Status status;
	AttrDesc* attrs = new AttrDesc[projCnt];
	AttrDesc ad;
	const char *filter;
	int width = 0;
	
	// Get all attribute descriptions of each projName
	for(int i = 0; i < projCnt; i++){
	    if((status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, attrs[i])) != OK){ 
			delete[] attrs;
			return status;
		}
		width += attrs[i].attrLen;
    }
	
	if (attr != NULL){
		
		// Get and store the AttrDesc for the relName and attrName of the attr
		if((status = attrCat->getInfo(attr->relName, attr->attrName, ad)) != OK) { 
			delete[] attrs;
			return status;
		}
		
		// get the attribute type and store it as a filter
		float tempf;
		int tempi;
		switch(ad.attrType){
			case FLOAT:
			
			    tempf = atof(attrValue);
			    filter = (char *)&tempf;
			    break;
				
		    case INTEGER:
			
			    tempi = atoi(attrValue);
			    filter = (char *)&tempi;
			    break;
				
		    default:
			    filter = attrValue;
			    break;
	    }
		
		// perform the ScanSelect with the correct filter
		if((status = ScanSelect(result, projCnt, attrs, &ad, op, filter, width)) != OK){ 
			delete[] attrs;
			return status;
		}
		
	} else {
		
		// attr is null so we do an unconditional scan
		if((status = ScanSelect(result, projCnt, attrs, NULL, op, NULL, width)) != OK) { 
			delete[] attrs;
			return status;
		}
		
	}
	
	delete[] attrs;
	return OK;
	
	
	
}


const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;
    
	Status status;
	RID rid;
	RID temprid;
	Record rec;
	
	InsertFileScan* ifs = new InsertFileScan(result, status);
	HeapFileScan* hfs;
	if (attrDesc != NULL){
		hfs = new HeapFileScan(attrDesc->relName, status);
	} else {
		if (projCnt > 0){
			hfs = new HeapFileScan(projNames[0].relName, status);
		}
	}
    if(status != OK) {
		delete ifs;
		delete hfs;
		return status;
	}
	
	if (filter == NULL){
		if ((status = hfs->startScan(0, 0, (Datatype)attrDesc->attrType, filter, op)) != OK){
			delete ifs;
			delete hfs;
			return status;
		}
	} else {
		if ((status = hfs->startScan(attrDesc->attrOffset, reclen, (Datatype)0, filter, op)) != OK){
			delete ifs;
			delete hfs;
			return status;
		}
	}
	
	while((status = hfs->scanNext(rid)) == OK){
		
		if((status = hfs->getRecord(rec)) != OK) {
			delete ifs;
			delete hfs;
			return status;
		}
		
		
		
		
		if((status = ifs->insertRecord(rec, temprid)) != OK) {
			delete ifs;
			delete hfs;
			return status;
		}
		
	}
}
