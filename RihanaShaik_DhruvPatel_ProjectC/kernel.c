//Rihana Shaik and Dhruv Patel

    void printString(char* str);
    void printChar(char c);
    void readFile(char* fileName, char* buffer, int* bytesRead);
    void readString(char* str);
    void readSector(char* buffer, int sector);
    void handleInterrupt21(int ax, int bx, int cx, int dx);
    void printNewLine();
    void executeProgram(char* name);
    void terminate();

    void main() {
        char inputLine[80];
        char sectorBuffer[512];
        char buffer[13312];   /Maximum size of a file/
        int sectorsRead;

        makeInterrupt21();
        
        interrupt(0x21, 3, "messag", buffer, &sectorsRead);   /read the file into buffer/ 
        if (sectorsRead>0)
        interrupt(0x21, 0, buffer, 0, 0);   /print out the file/ 
        else
        interrupt(0x21, 4, "shell");
        
        while(1);   /hang up/ 
    }

    void terminate(){
        char shellname[6]; 
        shellname[0] ='s';
        shellname[1]='h'; 
        shellname[2]='e';
        shellname[3]='l'; 
        shellname[4]='l'; 
        shellname[5]='\0';  
        executeProgram(shellname); 
    }

    void readFile(char* fileName, char* buffer, int* bytesRead) {
        char directory[512];

        int fileEntrySize=32;  
        int directorySize = 512;  
        int fileEntry =0;
        int ffound = 0;
        int i, sector;
        readSector(directory, 2);  

        for (fileEntry = 0; fileEntry < directorySize; fileEntry += fileEntrySize) {
            for (i = 0; i < 6; i++) {
                if (fileName[i] != directory[fileEntry + i]) {
                    break;
                }
            }
            if (i == 6) {
                ffound = 1;
                break;  
            }
        }

        if (!ffound) {
            *bytesRead = 0;  
            return;
        }

        sector = directory[fileEntry + 6];
        *bytesRead = 0;
        while (sector != 0) {
            readSector(buffer, sector);  
            buffer += 512;  
            sector = directory[fileEntry + 7];  
            (*bytesRead)++;
        }
    }

    void printString(char* str) {
        while (*str != '\0') {
            printChar(*str);
            str += 0x1;
        }
    }

    void printChar(char c) {
        int ah, al, ax;
        ah = 0xe;
        al = c;
        ax = (ah * 256) + al;
        interrupt(0x10, ax, 0, 0, 0);
    }

    void readString(char* str) {
        char* start;
        char c;
        for (c = interrupt(0x16, 0, 0, 0, 0); c != 0xd; c = interrupt(0x16, 0, 0, 0, 0)) {
            if (c == 0x8) {
                if (str != start) {
                    printChar(0x8);
                    printChar(0x20);
                    printChar(0x8);
                    str -= 0x1;
                    *str = 0x20;
                    }
                } 
            else {
                printChar(c);
                *str = c;
                str += 0x1;
                }
        }

        printChar(c);
        printChar(0xa);
        *str = 0xa;
        str += 0x1;
        *str = 0x0;
    }

    void readSector(char* buffer, int sector) {
        int ah, al, bx, ch, cl, dh, dl, ax, cx, dx;
        ah = 2;
        al = 1;
        bx = buffer;
        ch = 0; 
        cl = sector + 1; 
        dh = 0; 
        dl = 0x80;

        ax = (ah * 256) + al;
        cx = (ch * 256) + cl;
        dx = (dh * 256) + dl;
        buffer = interrupt(0x13, ax, bx, cx, dx);
    }

    void handleInterrupt21(int ax, int bx, int cx, int dx) {
        switch(ax) {
            case 0:
                printString(bx);
                break;
            case 1:
                readString(bx);
                break;
            case 2:
                readSector(bx, cx);
                break;
            case 3:
                readFile(bx, cx, dx);
                break;
            case 4:
                executeProgram(bx);
                break;
            case 5:
                terminate();
                break;
        default:
                printString("Invalid ax");
        }

    }


    void executeProgram(char* name) {
        char buffer[512 * 10];  
        int segment = 0x2000;
        int offset = 0;
        int sectorsRead,i;
        readFile(name, buffer, &sectorsRead);

        if (sectorsRead == 0) {
            printString("Program not found.\n");
            return;
        }

        for (i = 0; i < 512 * sectorsRead; ++i) {
            putInMemory(0x2000, offset, buffer[i]);
            ++offset;
            if (offset == 0x1000) {
                offset = 0;
                ++segment;
            }
        }

    
        launchProgram(0x2000);
    
    }
