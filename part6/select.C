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
	
	// Get all attribute descriptions of each projName and calculate width of result record.
	for(int i = 0; i < projCnt; i++){
	    if((status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, attrs[i])) != OK){ 
			delete[] attrs;
			return status;
		}
		width += attrs[i].attrLen;
    }
	
	//If we get an attribute info object to use as the filter
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
		//No filtering attribute provided, need a relname for the HeapFileScan
		// so we retrieve the attrDesc for the first object in the projection
		// attribute array
		
		if (projCnt > 0){
			if((status = attrCat->getInfo( projNames[0].relName, projNames[0].attrName, ad)) != OK) {
				delete[] attrs;
				return status;
			}
		}
		//No attributes to use in the projection, return OK to prevent waste of CPU time
		else {
			return OK;
		}
		
		// attr is null so we do an unconditional scan
		if((status = ScanSelect(result, projCnt, attrs, &ad, op, NULL, width)) != OK) { 
			delete[] attrs;
			return status;
		}
		
	}
	
	//final cleanup
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
	Record resultRec;
	
	InsertFileScan* ifs = new InsertFileScan(result, status);
	HeapFileScan* hfs;
	hfs = new HeapFileScan(attrDesc->relName, status);
	
	if(status != OK) {
		delete ifs;
		delete hfs;
		return status;
	}
	

	if (filter == NULL){
		//Start unconditional scan
		if ((status = hfs->startScan(0, 0, (Datatype)attrDesc->attrType, filter, op)) != OK){
			delete ifs;
			delete hfs;
			return status;
		}
	} else {
		//Start scan using a filter
		if ((status = hfs->startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype)attrDesc->attrType, filter, op)) != OK){
			delete ifs;
			delete hfs;
			return status;
		}
	}
	
	//Prepare result record for the results table
	char outputData[reclen];
	resultRec.data = (void *)outputData;
	resultRec.length = reclen;
	
	
	while((status = hfs->scanNext(rid)) == OK){
		
		if((status = hfs->getRecord(rec)) != OK) {
			delete ifs;
			delete hfs;
			return status;
		}
		
		float tempf;
		int tempi;
		int outOffset = 0;
		
		//Found a record matching our scan constraints, copy data from found record into
		// the result record.  Keep track of where we are inserting into the results record
		// by using the outOffset which is incremented by the attrLen of each attr added.
		for (int i = 0; i < projCnt; i++) {

			memcpy(outputData + outOffset, (char *)rec.data + projNames[i].attrOffset,
							projNames[i].attrLen);
			   
			   
			   
			outOffset += projNames[i].attrLen;
		}
		
		//Insert result record into results table
		if((status = ifs->insertRecord(resultRec, temprid)) != OK) {
			delete ifs;
			delete hfs;
			return status;
		}
		
	}
	
	//final cleanup
	delete hfs;
	delete ifs;
	
	if( status != FILEEOF ) return status;
	else return OK;
}
