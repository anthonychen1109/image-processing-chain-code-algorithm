#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class image{
    friend class chainCode;
    
    int numRows, numCols, minVal, maxVal;
    int** zeroFramedAry;
public:
    image(int numRows, int numCols, int minVal, int maxVal){
        this->numRows = numRows;
        this->numCols = numCols;
        this->minVal = minVal;
        this->maxVal = maxVal;
        
        //dynamically allocate zeroFramedAry
        zeroFramedAry = new int*[numRows+2];
        for(int i=0; i<numRows+2; i++){
            zeroFramedAry[i] = new int[numCols+2];
        }//for i
        
        //initialize zeroFramedAry to 0
        for(int i=0; i<numRows+2; i++){
            for(int j=0; j<numCols+2; j++){
                zeroFramedAry[i][j] = 0;
            }//for j
        }//for i
    }//constructor
    
    void loadImage(ifstream& inFile){
        int data;
        for(int i=1; i<numRows+1; i++){
            for(int j=1; j<numCols+1; j++){
                while(!inFile.eof()){
                    inFile >> data;
                    zeroFramedAry[i][j] = data;
                    break;
                }//while
            }//for j
        }//for i
        inFile.close();
    }//loadImage
    
};//image class

class CCproperty{
    friend class chainCode;
    //private:
public:
    int maxCC;
    
    struct Property{
        int label, numPixels, minRow, minCol, maxRow, maxCol;
    };//struct Property
    Property* table;
    
    CCproperty(int maxCC){
        int size = maxCC + 1;
        table = new Property[size];
    }//constructor
    
    //read properties from inFile to prepare to load into 1D struct
    void getProperties(ifstream& inFile, int size){
        int label, numPixels, minRow, minCol, maxRow, maxCol;
        for(int i=1; i<=size; i++){
            inFile >> label;
            inFile >> numPixels;
            inFile >> minRow;
            inFile >> minCol;
            inFile >> maxRow;
            inFile >> maxCol;
            loadProperties(i, numPixels, minRow, minCol, maxRow, maxCol);
        }//for i
        inFile.close();
    }//getProperties
    
    //stores properties into 1D struct after reading
    void loadProperties(int numLabel, int numPixels, int minRow, int minCol, int maxRow, int maxCol){
        int label = numLabel;
        table[label].label = label;
        table[label].numPixels = numPixels;
        table[label].minRow = minRow;
        table[label].minCol = minCol;
        table[label].maxRow = maxRow;
        table[label].maxCol = maxCol;
    }//loadProperties
    
    int getLabel(int numLabel){
        int label = table[numLabel].label;
        //cout << label;
        return label;
    }//getLabel
    
    int getNumPixels(int numLabel){
        int numPixels = table[numLabel].numPixels;
        //cout << numPixels;
        return numPixels;
    }//getNumPixels
    
    int getMinRow(int numLabel){
        int minRow = table[numLabel].minRow;
        //cout << "minRow: " << minRow << " ";
        return minRow;
    }//getMinRow
    
    int getMinCol(int numLabel){
        int minCol = table[numLabel].minCol;
        //cout << "minCol: " << minCol << " ";
        return minCol;
    }//getMinRow
    
    int getMaxRow(int numLabel){
        int maxRow = table[numLabel].maxRow;
        //cout << "maxRow: " << maxRow << " ";
        return maxRow;
    }//getMinRow
    
    int getMaxCol(int numLabel){
        int maxCol = table[numLabel].maxCol;
        //cout << "maxCol: " << maxCol << " ";
        return maxCol;
    }//getMinRow
    
};//CCproperty class

class chainCode{
    //private:
public:
    int currentCC;
    int minRowOffset;
    int maxRowOffset;
    int minColOffset;
    int maxColOffset;
    int nextDirTable[8];// You may hard code this table as given in class
    int nextQ;// the next scanning direction of currentP's neighbors, range from 0 to 7, need to mod 8.
    int Pchain;// chain code direction from currentP to nextP
    int lastQ;// Q, range from 0 to 7, it is the direction of the last zero scanned from P,
    
    struct Point{
        int row, col;
    }neighborCoord[8],
    currentP,
    nextP; //P'
    //struct Point
    
    chainCode(){
        currentCC = 0;
        
        nextDirTable[0] = 6;
        nextDirTable[1] = 0;
        nextDirTable[2] = 0;
        nextDirTable[3] = 2;
        nextDirTable[4] = 2;
        nextDirTable[5] = 4;
        nextDirTable[6] = 4;
        nextDirTable[7] = 6;
    }//constructor
    
    void loadNeighborsCoord(Point currentP){
        int i = currentP.row;
        int j = currentP.col;
        
        neighborCoord[0].row = i;
        neighborCoord[0].col = j+1;
        neighborCoord[1].row = i-1;
        neighborCoord[1].col = j+1;
        neighborCoord[2].row = i-1;
        neighborCoord[2].col = j;
        neighborCoord[3].row = i-1;
        neighborCoord[3].col = j-1;
        neighborCoord[4].row = i;
        neighborCoord[4].col = j-1;
        neighborCoord[5].row = i+1;
        neighborCoord[5].col = j-1;
        neighborCoord[6].row = i+1;
        neighborCoord[6].col = j;
        neighborCoord[7].row = i+1;
        neighborCoord[7].col = j+1;
    }//loadNeighborsCoord
    
    int findNextP(image img, Point currentP, int nextQ, int currentCC){
        loadNeighborsCoord(currentP);
        nextQ = (lastQ+1) % 8;
        int chainDir = nextQ;
        for(int i=chainDir; i<chainDir+8;i++){
            if(img.zeroFramedAry[neighborCoord[i%8].row][neighborCoord[i%8].col] == currentCC){
                chainDir = i%8;
            }//if
        }//for
        nextP = neighborCoord[chainDir];
        return chainDir;
    }//findNextP
    
    void computeChainCode(ifstream& inFile, image img, CCproperty cc, ofstream& outFile, int size){
        currentCC = 0;
        Point startP;
        int maxCC = size+1;
        
        while(currentCC < maxCC){
            currentCC++;
            minRowOffset = cc.getMinRow(currentCC);
            maxRowOffset = cc.getMaxRow(currentCC);
            minColOffset = cc.getMinCol(currentCC);
            maxColOffset = cc.getMaxCol(currentCC);
            for(int i=minRowOffset+1; i<=maxRowOffset+1; i++){
                for(int j=minColOffset+1; j<=maxColOffset+1; j++){
                    if(img.zeroFramedAry[i][j] == currentCC){
                        startP.row = i;
                        startP.col = j;
                        currentP.row = i;
                        currentP.col = j;
                        lastQ = 4;
                        
                        nextQ = (lastQ+1) % 8;
                        Pchain = findNextP(img, currentP, nextQ, currentCC);
                        nextP = neighborCoord[Pchain];
                        outFile << "startRow: " << i << " " << "startCol: " << j << " " << "CC_Label: " << currentCC << " " << "code: " << Pchain << "\n";
                        outFile << "\n";
                        lastQ = nextDirTable[Pchain];
                        currentP = nextP;
                    }//if
                }//for j
            }//for i
            outFile << "\n";
        }
    }//computeChainCode
    
    void prettyPrint(image img, ofstream& out){
        for(int i=0; i<img.numRows+2; i++){
            for(int j=0; j<img.numCols+2; j++){
                if(img.zeroFramedAry[i][j] == 0){
                    out << " ";
                }//if
                else{
                    out << "1";
                }//else
            }//for j
        }//for i
    }//prettyPrint
    
};//chainCode class

int main(int argc, const char * argv[]) {
    ifstream inFile1;
    ifstream inFile2;
    ofstream outFile1;
    ofstream outFile2;
    
    inFile1.open(argv[1]);
    outFile1.open(argv[3]);
    
    int numRows, numCols, minVal, maxVal;
    inFile1 >> numRows;
    inFile1 >> numCols;
    inFile1 >> minVal;
    inFile1 >> maxVal;
    
    image myImage(numRows, numCols, minVal, maxVal);
    myImage.loadImage(inFile1);
    
    inFile2.open(argv[2]);
    
    int label = 0, numPixels, minRow, minCol, maxRow, maxCol;
    while(!inFile2.eof()){ //get the # of labels to load into properties
        inFile2 >> label;
        inFile2 >> numPixels;
        inFile2 >> minRow;
        inFile2 >> minCol;
        inFile2 >> maxRow;
        inFile2 >> maxCol;
    }//while
    inFile2.close();
    
    inFile2.open(argv[2]);
    CCproperty myProperty(label);
    myProperty.getProperties(inFile2, label);
    
    inFile1.open(argv[1]);
    inFile2.open(argv[2]);
    
    chainCode myChainCode;
    myChainCode.computeChainCode(inFile1,myImage,myProperty,outFile1,label);
    //myChainCode.prettyPrint(myImage, outFile2);
    
    outFile1.close();
    outFile2.close();
    return 0;
}//main

