//
// Created by jmeintrup on 02.05.18.
//
#include <cstdlib>
#include <cmath>
#include <iostream>
#include "sealib/localdycktable.h"

static unsigned long mask[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};


Sealib::LocalDyckTable::LocalDyckTable(unsigned char len) {
    entries = static_cast<unsigned long>(std::pow(2, len));
    table = static_cast<Data**>(malloc((sizeof(Data*) * entries)));

    for(unsigned long i = 0; i <= entries; i++) {
        table[i] = calculateLocalData(i, len);
    }
}

Sealib::LocalDyckTable::LocalDyckTable() : LocalDyckTable(7){
}

Sealib::LocalDyckTable::Data* Sealib::LocalDyckTable::calculateLocalData(unsigned long g, unsigned char len) {
    Data *d = new Data(len);

    for(unsigned char c = 0; c < len; c++) {
        d->localMatches[c] = c;
        d->localDepths[c] = 0;
    }

    d->rightPioneer = (unsigned char) - 1;
    d->leftPioneer = (unsigned char) - 1;

    unsigned char j = 0;
    unsigned char p = 0;

    auto *stack = static_cast<unsigned char *>(malloc((sizeof(unsigned char) * len)));
    while(j != len) {
        if((g & mask[j]) != 0) { // '('
            stack[p++] = j;
        } else {
            if (p > 0) { // j > stack[p]
                d->localMatches[j] = stack[--p];
                d->localMatches[stack[p]] = j;
                for(auto c = static_cast<unsigned char>(stack[p] + 1); c < j; c++) {
                    d->localDepths[c]++;
                }
            }
        }
        j++;
    }

    char openingDepth = 0;
    char closingDepth = 0;
    for(unsigned char c = 0; c < len; c++) {
        if(d->localMatches[c] == c && (g & mask[c]) != 0) { //left most global opening
            /*if(openingDepth == 0) {
                d->leftPioneer = c;;
            }*/
            d->localDepths[c] = ++openingDepth;
        } else if(d->localMatches[len - c - 1] == len - c - 1 && (g & mask[len - c - 1]) == 0) { //right most global closing
            /*if(closingDepth == 0) {
                d->rightPioneer = static_cast<unsigned char>(len - c - 1);;
            }*/
            d->localDepths[len - c - 1] = --closingDepth;
        }
    }
    for(unsigned char c = 0; c < len; c++) {
        if(d->localMatches[c] == c && (g & mask[c]) != 0) {
            d->leftPioneer = c;
            break;
        }
    }

    for(int c = len - 1; c >= 0; c--) {
        if(d->localMatches[c] == c && (g & mask[c]) == 0) {
            d->rightPioneer = static_cast<unsigned char>(c);
            break;
        }
    }
    return d;
}

const Sealib::LocalDyckTable::Data* Sealib::LocalDyckTable::operator[](unsigned long i) {
    return table[i];
}

Sealib::LocalDyckTable::~LocalDyckTable() {
    for(unsigned long i = 0; i < entries; i++) {
        delete(table[i]);
    }
    free(table);
}

unsigned long Sealib::LocalDyckTable::getEntries() const {
    return entries;
}

Sealib::LocalDyckTable::Data::Data(unsigned char len) {
    localMatches = static_cast<unsigned char *>(malloc(sizeof(char) * len));
    localDepths = static_cast<char *>(malloc(sizeof(char) * len));
    leftPioneer = (unsigned char) - 1;
    rightPioneer = (unsigned char) - 1;
    for(unsigned char c = 0; c < len; c++) {
        localMatches[c] = c;
        localDepths[c] = 0;
    }
}

Sealib::LocalDyckTable::Data::~Data() {
    free(localMatches);
    free(localDepths);
}