#include "SoloFeature.h"
#include "streamFuns.h"
#include "TimeFunctions.h"
#include "serviceFuns.cpp"
#include "SequenceFuns.h"
#include "ErrorWarning.h"
#include "SoloFeatureTypes.h"

void SoloFeature::outputResults(bool cellFilterYes, string outputPrefixMat)
{    
    //make directories   
    createDirectory(outputPrefixMat,P.runDirPerm, "Solo output directory", P);
    /* old way, does not work when parent directores are needed
    if (mkdir(outputPrefixMat.c_str(),P.runDirPerm)!=0 && errno!=EEXIST) {//create directory
        exitWithError("EXITING because of fatal OUTPUT FILE error: could not create Solo output directory" + outputPrefixMat + 
                      "\nSOLUTION: check the path and permisssions\n",
                       std::cerr, P.inOut->logMain, EXIT_CODE_PARAMETER, P);
    };
    */
    
    /////////////////////////////////////////////////////////////
    //write features.tsv
    switch (featureType) {
        case SoloFeatureTypes::Gene :
        case SoloFeatureTypes::GeneFull :
        case SoloFeatureTypes::Velocyto :
        case SoloFeatureTypes::VelocytoSimple :
        {
            ofstream &geneStr=ofstrOpen(outputPrefixMat+pSolo.outFileNames[1],ERROR_OUT, P);
            for (uint32 ii=0; ii<Trans.nGe; ii++) {
                geneStr << Trans.geID[ii] <<"\t"<< (Trans.geName[ii].empty() ? Trans.geID[ii] : Trans.geName[ii]);
				if (pSolo.outFormat.featuresGeneField3!="-") {
					geneStr <<'\t'<< pSolo.outFormat.featuresGeneField3;
				};
				geneStr << '\n';
            };
            geneStr.close();
            break;
        };
        case SoloFeatureTypes::SJ :
        	symlink("../../../SJ.out.tab", (outputPrefixMat+pSolo.outFileNames[1]).c_str());
    };

    ////////////////////////////////////////////////////////////////////////////
    //write barcodes.tsv
    ofstream &cbStr=ofstrOpen(outputPrefixMat+pSolo.outFileNames[2],ERROR_OUT, P);
    uint64 nCellGeneEntries=0;//total number of non-zero cell/gene combinations (entries in the output matrix)
    if (cellFilterYes) {//filtered cells
        for (uint32 icb=0; icb<nCB; icb++) {
            if (filteredCells.filtVecBool[icb]) {
                cbStr << pSolo.cbWLstr[indCB[icb]] <<'\n';
                nCellGeneEntries += nGenePerCB[icb];
            };
        };
    } else {//unfiltered cells
        for (uint64 ii=0; ii<pSolo.cbWLsize; ii++)
             cbStr << pSolo.cbWLstr[ii] <<'\n';
        for (uint32 icb=0; icb<nCB; icb++) {
            nCellGeneEntries += nGenePerCB[icb];
        };
    };
    cbStr.flush();   

    /////////////////////////////////////////////////////////////
    //output counting matrix
    string matrixFileName=outputPrefixMat+pSolo.outFileNames[3];
    ofstream &countMatrixStream=ofstrOpen(matrixFileName,ERROR_OUT, P);
    
    
    vector <uint32> umiOutCol = pSolo.umiDedupColumns;
    if ( featureType==SoloFeatureTypes::Velocyto || featureType==SoloFeatureTypes::VelocytoSimple ) {
        umiOutCol={0,1,2};
    } else if ( pSolo.umiDedup[0]=="1MM_CR" ) {//only one column allowed
        umiOutCol={0};
    };

    //header
    countMatrixStream <<"%%MatrixMarket matrix coordinate integer general\n%\n";
    countMatrixStream << featuresNumber <<' '<< (cellFilterYes ? filteredCells.nCells : pSolo.cbWLsize) <<' '<< nCellGeneEntries << '\n';
    
    uint32  cbInd1=0;
    for (uint32 icb=0; icb<nCB; icb++) {
        if (cellFilterYes) {
            if (filteredCells.filtVecBool[icb]) {
                ++cbInd1;
            } else {
                continue;
            };
        } else {
            cbInd1=indCB[icb]+1;
        };
        for (uint32 ig=0; ig<nGenePerCB[icb]; ig++) {
            
            uint32 indG1=countCellGeneUMIindex[icb]+ig*countMatStride;
            
            //feature index, CB index
            countMatrixStream << countCellGeneUMI[indG1]+1 <<' '<< cbInd1;

            for (uint32 ii=0; ii<umiOutCol.size(); ii++)
                countMatrixStream <<' '<< countCellGeneUMI[indG1+1+umiOutCol[ii]];
            
            countMatrixStream << '\n';
        };
    };
    countMatrixStream.flush();
};
