/*
 Easter Egg: Lightweight text editor for use with the ThinkGridWorldShell itself.

 Much of this code was based directly on the text editor example from source code. 
 https://viewsourcecodquarkorg/snaptoken/kilo/. 
*/

#define _DEFAULT_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#include "quark.h"

/* Syntax Highlighting ------------------------------------------------------*/
/* 
Utility function that indicates whether or not a given char is a separator.
Inputs:
    int s : Character to check.
Outputs:
    0     : False, if character is not a separator.
    1     : True, if character is a separator.
*/
int isSeparator(int s) {
    return s == '\0' || isspace(s) || strchr(",.()+-/*=~%[];",s) != NULL;
}

/* 
Returns whether or not a line has a multiline comment. Taken directly
from tutorial bescause other attempts failed - still unstable.
Inputs:
    *row : Row to assess whether or not it has a comment.
Outputs:
    1    : True, so if there is a multi-line comment. 
    0    : False, so if there is no multi-line comment.
*/
int isMultiline(erow *row) {
    if (row->hl && row->rsize && row->hl[row->rsize-1] == HL_MLCOMMENT &&
        (row->rsize < 2 || (row->render[row->rsize-2] != '*' ||
                            row->render[row->rsize-1] != '/'))) return 1;
    return 0;
}

/* 
Sets every byte to each character of the line for each byte to the
highlighting type. Tutorial explains why this is necessary, code was 
very heavily taken from there.
Inputs:
    *row : Row to asssess highlighting type for.
Output:
    VOID
*/
void setCharByte(erow *row) {
    if (quark.syntax == NULL) return;
    row->hl = realloc(row->hl,row->rsize);
    memset(row->hl,HL_NORMAL,row->rsize);

    int offset, prev_sep, in_string, in_comment;
    char *p;
    char **keywords = quark.syntax->keywords;
    char *scs = quark.syntax->singleline_comment_start;
    char *mcs = quark.syntax->multiline_comment_start;
    char *mce = quark.syntax->multiline_comment_end;

    /* Find the first character that is not a space. */
    p = row->render;
    /* Offset of the current character. */
    offset = 0; 
    while(*p && isspace(*p)) {
        p++;
        offset++;
    }
    /* If the pointer is to the start. */
    prev_sep = 1;
    /* If we are inside quotation marks. */
    in_string = 0;
    /* If we are inside a comment. */
    in_comment = 0;

    /* Account for if we are within an open comment. */
    if (row->idx > 0 && isMultiline(&quark.row[row->idx-1]))
        in_comment = 1;

    while(*p) {
        /* If comments are marked by //. */
        if (prev_sep && *p == scs[0] && *(p+1) == scs[1]) {
            memset(row->hl+offset,HL_COMMENT,row->size-offset);
            return;
        }

        /* Multi-line comments. */
        if (in_comment) {
            row->hl[offset] = HL_MLCOMMENT;
            if (*p == mce[0] && *(p+1) == mce[1]) {
                row->hl[offset+1] = HL_MLCOMMENT;
                p += 2; offset += 2;
                in_comment = 0;
                prev_sep = 1;
                continue;
            } else {
                prev_sep = 0;
                p++; offset++;
                continue;
            }
        } else if (*p == mcs[0] && *(p+1) == mcs[1]) {
            row->hl[offset] = HL_MLCOMMENT;
            row->hl[offset+1] = HL_MLCOMMENT;
            p += 2; offset += 2;
            in_comment = 1;
            prev_sep = 0;
            continue;
        }

        /* If we have quotation marks. */
        if (in_string) {
            row->hl[offset] = HL_STRING;
            if (*p == '\\') {
                row->hl[offset+1] = HL_STRING;
                p += 2; offset += 2;
                prev_sep = 0;
                continue;
            }
            if (*p == in_string) in_string = 0;
            p++; offset++;
            continue;
        } else {
            if (*p == '"' || *p == '\'') {
                in_string = *p;
                row->hl[offset] = HL_STRING;
                p++; offset++;
                prev_sep = 0;
                continue;
            }
        }

        /* If we cannot print a specific character. */
        if (!isprint(*p)) {
            row->hl[offset] = HL_NONPRINT;
            p++; offset++;
            prev_sep = 0;
            continue;
        }

        /* If we have a number, */
        if ((isdigit(*p) && (prev_sep || row->hl[offset-1] == HL_NUMBER)) ||
            (*p == '.' && offset>0 && row->hl[offset-1] == HL_NUMBER)) {
            row->hl[offset] = HL_NUMBER;
            p++; offset++;
            prev_sep = 0;
            continue;
        }

        /* If we have keywords or calls to libraries. */
        if (prev_sep) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen-1] == '|';
                if (kw2) klen--;

                if (!memcmp(p,keywords[j],klen) &&
                    isSeparator(*(p+klen)))
                {

                    memset(row->hl+offset,kw2 ? HL_KEYWORD2 : HL_KEYWORD1,klen);
                    p += klen;
                    offset += klen;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_sep = 0;
                continue; 
            }
        }

        /* Check for separators and special characters. */
        prev_sep = isSeparator(*p);
        p++; offset++;
    }

    /* Account for multiline comments. */
    int oc = isMultiline(row);
    if (row->hl_oc != oc && row->idx+1 < quark.numrows)
        setCharByte(&quark.row[row->idx+1]);
    row->hl_oc = oc;
}

/* 
Syntax highlighting matching function. Values taken from tutorial code.
Inputs:
    int lineType : Corresponds to type of syntax used.
Outputs:
    int associated with rgb color value.
*/
int determineColor(int lineType) {
    switch(lineType) {
    case HL_COMMENT:
    case HL_MLCOMMENT: return 36;   /* Cyan. */
    case HL_KEYWORD1: return 33;    /* Yellow. */
    case HL_KEYWORD2: return 32;    /* Green. */
    case HL_STRING: return 35;      /* Magenta. */
    case HL_NUMBER: return 31;      /* Red. */
    case HL_MATCH: return 34;       /* Blue. */
    default: return 37;             /* White. */
    }
}

/* 
Uses the name of the file to determine how to do syntax highlighting.
Updated the quark.syntax value with given name. Taken from tutorial.
Inputs:
    char* f : Name of file to parse name of.
Outputs:
    void
*/
void determineHighlight(char *f) {
    for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
        struct editorSyntax *e = HLDB+j;
        unsigned int i = 0;
        while(e->filematch[i]) {
            char *p;
            int patlen = strlen(e->filematch[i]);
            if ((p = strstr(f,e->filematch[i])) != NULL) {
                if (e->filematch[i][0] != '.' || p[patlen] == '\0') {
                    quark.syntax = e;
                    return;
                }
            }
            i++;
        }
    }
}
/* Escape Sequences ---------------------------------------------------------*/
/*
Reads keys from input keys and looks for escape sequences.
Input:	
    int f : File process instance.
Output:	
    integer key value.
*/
int quarkReadKey(int f) {
    int nread;
    char s, seq[3];
    while ((nread = read(f,&s,1)) == 0);
    if (nread == -1) exit(1);
	/* 
	    Used tutorial descriptions for actual setup 
	    of escape sequences. 
	*/
    while(1) {
        switch(s) {
        case ESC:
            if (read(f,seq,1) == 0) return ESC;
            if (read(f,seq+1,1) == 0) return ESC;
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    if (read(f,seq+2,1) == 0) return ESC;
                    if (seq[2] == '~') {
                        switch(seq[1]) {
                        case '3': return DEL_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        }
                    }
                } else {
                    switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                    }
                }
            }
            else if (seq[0] == 'O') {
                switch(seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                }
            }
            break;
        default:
            return s;
        }
    }
}

/* 
Find the cursor position, report it, and returns it.
Inputs:
	i_f : Input process instance.
	o_f : Output process instance.
	*r  : Pointer to location to store row value of cursor.
	*c  : Pointer to location to store col value of cursor.
Outputs:
	-1  : Error
	0   : If position of cursor has been stored at *rows and *cols.
*/
int getCursorPosition(int i_f, int o_f, int *r, int *c) {
    char buf[32];
    unsigned int i = 0;
    if (write(o_f, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf)-1) {
        if (read(i_f,buf+i,1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != ESC || buf[1] != '[') return -1;
    if (sscanf(buf+2,"%d;%d",r,c) != 2) return -1;
    return 0;
}

/* 
Tries to find the window size and report it.
Inputs:
	i_f : Input process instance.
	o_f : Output process instance.
	*r  : Pointer to location to store row value of winsize.
	*c  : Pointer to location to store col value of winsize.
Outputs:
	-1 : Error
	0  : If winsize has been stored at *rows and *cols.
*/
int getWindowSize(int i_f, int o_f, int *r, int *c) {
 	struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        int prev_r, prev_c, val;

        /* Start with storing the initial position. */
        val = getCursorPosition(i_f,o_f,&prev_r,&prev_c);
        if (val == -1) {
            return -1;
        }

        /* Get the poisition of the bottom right corner. */
        if (write(o_f,"\x1b[999C\x1b[999B",12) != 12) {
			return -1;
		}
        val = getCursorPosition(i_f,o_f,r,c);
        if (val == -1) {
			return -1;
		}

        /* Go back to the original position. */
        char seq[32];
        snprintf(seq,32,"\x1b[%d;%dH",prev_r,prev_c);
        if (write(o_f,seq,strlen(seq)) == -1) {
			return -1;
        }
        return 0;
    } else {
        *c = ws.ws_col;
        *r = ws.ws_row;
        return 0;
    }
}
/* Actual editor window --------------------------------------------------------*/


/* 
Update individual rows based on tutorial.
Inputs:
    errow *row : Points to update row.
Outputs:
    void
*/
void updateRow(erow *row) {
    int tabs = 0, nonprint = 0, j, idx;
    /* Creates row we can free. */
    free(row->render);
    for (j = 0; j < row->size; j++)
        if (row->chars[j] == TAB) tabs++;
    row->render = malloc(row->size + tabs*8 + nonprint*9 + 1);
    idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == TAB) {
            row->render[idx++] = ' ';
            while((idx+1) % 8 != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->rsize = idx;
    row->render[idx] = '\0';

    /* Highlight syntax! */
    setCharByte(row);
}

/* 
Insert a row at the position requested and move the rows below if needed. Outputs void.
Inputs:
    int num    : Index to add a row at.
    char* s    : Characters to add at row.
    size_t len : Size of row to add.
*/
void insertRow(int num, char* s, size_t len) {
    if (num > quark.numrows) return;
    quark.row = realloc(quark.row,sizeof(erow)*(quark.numrows+1));
    if (num != quark.numrows) {
        memmove(quark.row+num+1,quark.row+num,sizeof(quark.row[0])*(quark.numrows-num));
        for (int j = num+1; j <= quark.numrows; j++) quark.row[j].idx++;
    }
    quark.row[num].size = len;
    quark.row[num].chars = malloc(len+1);
    memcpy(quark.row[num].chars,s,len+1);
    quark.row[num].hl = NULL;
    quark.row[num].hl_oc = 0;
    quark.row[num].render = NULL;
    quark.row[num].rsize = 0;
    quark.row[num].idx = num;
    updateRow(quark.row+num);
    quark.numrows++;
    quark.unsaved++;
}

/*
Frees a given row and its attributes.
Input:
    errow *row : Pointer to the row to free. 
*/
void freeRow(erow *row) {
    free(row->render);
    free(row->chars);
    free(row->hl);
}

/* 
Deletes the row at a specific position and returns void.
Input:
    int num : Index of row to delete values at.
*/
void deleteRow(int num) {
    erow *r;
    if (num >= quark.numrows) return;
    r = quark.row+num;
    freeRow(r);
    memmove(quark.row+num,quark.row+num+1,sizeof(quark.row[0])*(quark.numrows-num-1));
    for (int j = num; j < quark.numrows-1; j++) quark.row[j].idx++;
    quark.numrows--;
    quark.unsaved++;
}

/* Turn the editor rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, escluding
 * the final nulterm. */
/* 
Turns all of the rows into a single string. 
Input:
    int* buflen : Pointer for the size of the string without the null.
Output:
    char *      : Heap-allocated string. Populates input buflen.
*/
char* createString(int* buflen) {
    char *buf = NULL, *p;
    int len = 0;
    int j;

    /* Count total numer of bytes. */
    for (j = 0; j < quark.numrows; j++)
        len += quark.row[j].size+1; /* Nominally considers null. */
    *buflen = len;
    len++; /* Adds to total length */

    p = buf = malloc(len);
    for (j = 0; j < quark.numrows; j++) {
        memcpy(p, quark.row[j].chars, quark.row[j].size);
        p += quark.row[j].size;
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}

/* 
Insert a character at the specified position in a row and move others.
Inputs:
    errow* row : Row to add the point to.
    int num    : Position to add the character to.
    int c      : Character to add.
*/
void insertCharacterAtRow(erow* row, int num, int c) {
    if (num > row->size) {
        /* Pad the string with spaces if the insert location is outside the
         * current length by more than a single character. */
        int padlen = num-row->size;
        /* In the next line +2 means: new char and null term. */
        row->chars = realloc(row->chars,row->size+padlen+2);
        memset(row->chars+row->size,' ',padlen);
        row->chars[row->size+padlen+1] = '\0';
        row->size += padlen+1;
    } else {
        /* If we are in the middle of the string just make space for 1 new
         * char plus the (already existing) null term. */
        row->chars = realloc(row->chars,row->size+2);
        memmove(row->chars+num+1,row->chars+num,row->size-num+1);
        row->size++;
    }
    row->chars[num] = c;
    updateRow(row);
    quark.unsaved++;
}

/* 
Insert a character buffer at the end of the row.
Inputs:
    errow* row : Row to add to.
    char *s    : Characters to add to the row.
    size_t len : Length to add.
*/
void appendStringAtRow(erow* row, char* s, size_t len) {
    row->chars = realloc(row->chars,row->size+len+1);
    memcpy(row->chars+row->size,s,len);
    row->size += len;
    row->chars[row->size] = '\0';
    updateRow(row);
    quark.unsaved++;
}

/* 
Delete the character at some index at some row.
Inputs:
    errow* row : Row to delete a character from.
    num        : Index to delete row from.
*/
void rowDeleteCharacter(erow* row, int num) {
    if (row->size <= num) return;
    memmove(row->chars+num,row->chars+num+1,row->size-num);
    updateRow(row);
    row->size--;
    quark.unsaved++;
}

/* 
Insert a character at a given position based on the cursor. 
Input:
    int c : Character to insert.
*/
void insertCharacter(int c) {
    int filerow = quark.rowoff+quark.cy;
    int filecol = quark.coloff+quark.cx;
    erow *row = (filerow >= quark.numrows) ? NULL : &quark.row[filerow];

    /* Add empty rows if needed. */
    if (!row) {
        while(quark.numrows <= filerow)
            insertRow(quark.numrows,"",0);
    }
    row = &quark.row[filerow];
    insertCharacterAtRow(row,filecol,c);
    if (quark.cx == quark.screencols-1)
        quark.coloff++;
    else
        quark.cx++;
    quark.unsaved++;
}


/* 
Insert a newline. We have to split the line if we try to do this in the 
middle of the line. Thanks viewsourcecode for your help on this one.
*/
void insertNewline(void) {
    int filerow = quark.rowoff+quark.cy;
    int filecol = quark.coloff+quark.cx;
    erow *row = (filerow >= quark.numrows) ? NULL : &quark.row[filerow];

    if (!row) {
        if (filerow == quark.numrows) {
            insertRow(filerow,"",0);
             if (quark.cy == quark.screenrows-1) {
                quark.rowoff++;
             } else {
                quark.cy++;
            }        
        }
        quark.cx = 0;
        quark.coloff = 0;
        return;
    }
    /* Check to see if the cursor is the last character. */
    if (filecol >= row->size) filecol = row->size;
    if (filecol == 0) {
        insertRow(filerow,"",0);
    } else {
        /* When we are in the middle of the line. */
        insertRow(filerow+1,row->chars+filecol,row->size-filecol);
        row = &quark.row[filerow];
        row->chars[filecol] = '\0';
        row->size = filecol;
        updateRow(row);
    }
}
/* 
Delete character at current position.
*/
void deleteCharacter() {
    int frow = quark.rowoff+quark.cy;
    int fcol = quark.coloff+quark.cx;
    erow *row = (frow >= quark.numrows) ? NULL : &quark.row[frow];
    /* Checks for zero-index chases and then deletes character and shifts
       rows as needed accordingly. */
    if (!row || (fcol == 0 && frow == 0)) return;
    if (fcol == 0) {
        fcol = quark.row[frow-1].size;
        appendStringAtRow(&quark.row[frow-1],row->chars,row->size);
        deleteRow(frow);
        row = NULL;
        if (quark.cy == 0)
            quark.rowoff--;
        else
            quark.cy--;
        quark.cx = fcol;
        if (quark.cx >= quark.screencols) {
            int shift = (quark.screencols-quark.cx)+1;
            quark.cx -= shift;
            quark.coloff += shift;
        }
    } else {
        rowDeleteCharacter(row,fcol-1);
        if (quark.cx == 0 && quark.coloff)
            quark.coloff--;
        else
            quark.cx--;
    }
    if (row) updateRow(row);
    quark.unsaved++;
}

/* 
Open file specified.
Input:
    char* f : Pointer to filename of file to open.
Output:
    1       : Error. 
    0       : Success.
*/
int openFile(char* f) {
    FILE *fp;

    quark.unsaved = 0;
    free(quark.filename);
    quark.filename = strdup(f);

    fp = fopen(f,"r");
    if (!fp) {
        if (errno != ENOENT) {
            perror("Opening file");
            exit(1);
        }
        return 1;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t length;
    while((length = getline(&line,&linecap,fp)) != -1) {
        if (length && (line[length-1] == '\n' || line[length-1] == '\r'))
            line[--length] = '\0';
        insertRow(quark.numrows,line,length);
    }
    free(line);
    fclose(fp);
    quark.unsaved = 0;
    return 0;
}


/* Saves the current file to the device.
Output:
    0 : Successful save.
    1 : Error.
TODO : Create a function for writerror instead of lumping.
*/
int saveFile(void) {
    int length;
    char *buf = createString(&length);
    int f = open(quark.filename,O_RDWR|O_CREAT,0644);
    if (f == -1) {
        free(buf);
        if (f != -1) close(f);
        setStatus("Can't save! I/O error: %s",strerror(errno));
        return 1;
    }
    if (ftruncate(f,length) == -1) {
        free(buf);
        if (f != -1) close(f);
        setStatus("Can't save! I/O error: %s",strerror(errno));
        return 1;
    }
    if (write(f,buf,length) != length) {
        free(buf);
        if (f != -1) close(f);
        setStatus("Can't save! I/O error: %s",strerror(errno));
        return 1;
    }
    close(f);
    free(buf);
    quark.unsaved = 0;
    setStatus("%d bytes written on disk", length);
    return 0;
}

/* 
Appends event buffer.
Input:
    eventbuffer *eb : EventBuffer struct to add on.
    const char *s   : String to add on.
    len             : Total length to copy.
*/
void ebAppend(struct ebuf *eb, const char *s, int len) {
    char *new = realloc(eb->b,eb->len+len);

    if (new == NULL) return;
    memcpy(new+eb->len,s,len);
    eb->b = new;
    eb->len += len;
}
/* 
Frees event buffer.
Input:
    eventbuffer *eb : is freed
Outputs:
    void
*/
void ebFree(struct ebuf *eb) {
    free(eb->b);
}

/*
Write to the entire screen globally. Tutorial was key in implementation.
Inputs and outputs are both VOID. 
*/
void editorRefreshScreen(void) {
    int y;
    erow *r;
    char buf[32];
    struct ebuf eb = ebuf_INIT;
    /* Hide the cursor and move back to original place. */
    ebAppend(&eb,"\x1b[?25l",6); 
    ebAppend(&eb,"\x1b[H",3);
    /* Go through each row of the screen. */
    for (y = 0; y < quark.screenrows; y++) {
        int filerow = quark.rowoff+y;
        int welcomelen = 80;
        if (filerow >= quark.numrows) {
            if (quark.numrows == 0 && y == quark.screenrows/3) {
                char welcome[80];
                /* If we need to add padding. */
                int pd = (quark.screencols-welcomelen)/2;
                if (pd) {
                    ebAppend(&eb,"~",1);
                    pd--;
                }
                while(pd--) ebAppend(&eb," ",1);
                ebAppend(&eb,welcome,welcomelen);
            } else {
                ebAppend(&eb,"~\x1b[0K\r\n",7);
            }
            continue;
        }
       
        r = &quark.row[filerow];

        int len = r->rsize - quark.coloff;
        int current_color = -1;
        /* Go through columns. */
        if (len > 0) {
            if (len > quark.screencols) len = quark.screencols;
            char *c = r->render+quark.coloff;
            unsigned char *hl = r->hl+quark.coloff;
            int j;
            for (j = 0; j < len; j++) {
                if (hl[j] == HL_NONPRINT) {
                    char sym;
                    ebAppend(&eb,"\x1b[7m",4);
                    if (c[j] <= 26)
                        sym = '@'+c[j];
                    else
                        sym = '?';
                    ebAppend(&eb,&sym,1);
                    ebAppend(&eb,"\x1b[0m",4);
                } else if (hl[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        ebAppend(&eb,"\x1b[39m",5);
                        current_color = -1;
                    }
                    ebAppend(&eb,c+j,1);
                } else {
                    int color = determineColor(hl[j]);
                    if (color != current_color) {
                        char buf[16];
                        int clen = snprintf(buf,sizeof(buf),"\x1b[%dm",color);
                        current_color = color;
                        ebAppend(&eb,buf,clen);
                    }
                    ebAppend(&eb,c+j,1);
                }
            }
        }
        ebAppend(&eb,"\x1b[39m",5);
        ebAppend(&eb,"\x1b[0K",4);
        ebAppend(&eb,"\r\n",2);
    }

    /* Add statuses. */
    ebAppend(&eb,"\x1b[0K",4);
    ebAppend(&eb,"\x1b[7m",4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
        quark.filename, quark.numrows, quark.unsaved ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus),
        "%d/%d",quark.rowoff+quark.cy+1,quark.numrows);
    if (len > quark.screencols) len = quark.screencols;
    ebAppend(&eb,status,len);
    while(len < quark.screencols) {
        if (quark.screencols - len == rlen) {
            ebAppend(&eb,rstatus,rlen);
            break;
        } else {
            ebAppend(&eb," ",1);
            len++;
        }
    }
    ebAppend(&eb,"\x1b[0m\r\n",6);

    ebAppend(&eb,"\x1b[0K",4);
    int msglen = strlen(quark.statusmsg);
    if (msglen && time(NULL)-quark.statusmsg_time < 5)
        ebAppend(&eb,quark.statusmsg,msglen <= quark.screencols ? msglen : quark.screencols);

    /* Move the cursor back to its current position. */
    int j;
    int cx = 1;
    int filerow = quark.rowoff+quark.cy;
    erow *row = (filerow >= quark.numrows) ? NULL : &quark.row[filerow];
    if (row) {
        for (j = quark.coloff; j < (quark.cx+quark.coloff); j++) {
            if (j < row->size && row->chars[j] == TAB) cx += 7-((cx)%8);
            cx++;
        }
    }
    snprintf(buf,sizeof(buf),"\x1b[%d;%dH",quark.cy+1,cx);
    ebAppend(&eb,buf,strlen(buf));
    ebAppend(&eb,"\x1b[?25h",6); /* Show cursor. */
    write(STDOUT_FILENO,eb.b,eb.len);
    ebFree(&eb);
}

/* 
Set a status message at the bottom of the editor for fun and sport. 
Built-in methods make it really straightforward, which is good.
Input:
    Character buffer to set the status.
Output:
    void
*/
void setStatus(const char *fmt, ...) {
    va_list val;
    va_start(val,fmt);
    vsnprintf(quark.statusmsg,sizeof(quark.statusmsg),fmt,val);
    va_end(val);
    quark.statusmsg_time = time(NULL);
}


/* 
Move cursor position based on arrow key movements. 

Input:
    int dir : Arrow key direction.  
*/
void quarkMoveCursor(int dir) {
    int r = quark.rowoff+quark.cy;
    int c = quark.coloff+quark.cx;
    int len;
    erow *row = (r >= quark.numrows) ? NULL : &quark.row[r];

    // Go through each direction and adjust coordinates and positions. 
    switch(dir) {
    case ARROW_LEFT:
        if (quark.cx == 0) {
            if (quark.coloff) {
                quark.coloff--;
            } else {
                if (r > 0) {
                    quark.cx = quark.row[r-1].size;
                    quark.cy--;
                    if (quark.cx > quark.screencols-1) {
                        quark.cx = quark.screencols-1;
                        quark.coloff = quark.cx-quark.screencols+1;
                    }
                }
            }
        } else {
            quark.cx -= 1;
        }
        break;
    case ARROW_RIGHT:
        if (row && c < row->size) {
            if (quark.cx == quark.screencols-1) {
                quark.coloff++;
            } else {
                quark.cx += 1;
            }
        } else if (row && c == row->size) {
            quark.cx = 0;
            quark.coloff = 0;
            if (quark.cy == quark.screenrows-1) {
                quark.rowoff++;
            } else {
                quark.cy += 1;
            }
        }
        break;
    case ARROW_UP:
        if (quark.cy == 0) {
            if (quark.rowoff) quark.rowoff--;
        } else {
            quark.cy -= 1;
        }
        break;
    case ARROW_DOWN:
        if (r < quark.numrows) {
            if (quark.cy == quark.screenrows-1) {
                quark.rowoff++;
            } else {
                quark.cy += 1;
            }
        }
        break;
    }

    // Increase number of columns or rows as needed. 
    r = quark.rowoff+quark.cy;
    c = quark.coloff+quark.cx;
    row = (r >= quark.numrows) ? NULL : &quark.row[r];
    len = row ? row->size : 0;
    if (c > len) {
        quark.cx -= c-len;
        if (quark.cx < 0) {
            quark.coloff += quark.cx;
            quark.cx = 0;
        }
    }
}

/*
Processes individual keystrokes as the user types them. 
Input:
    int f : Process object.
Output:
    void.
*/
void editorKeyProcess(int f) {
    /* When the file is modified, requires Ctrl-q to be pressed N times
       before actually quitting. */
    int s = quarkReadKey(f);

    /* 
       Process individual cases. Thanks tutorial for what each keystroke
       could do to be a reasonable user experience! 
       It's not quite vim, but that's okay because it is easier to find
       the keystrokes with control.
    */
    switch(s) {
    case ENTER:
        insertNewline();
        break;
    case CTRL_C:
        break;
    case CTRL_Q:       
        exit(0);
        break;
    case CTRL_S: 
        saveFile();
        break;
    case BACKSPACE:    
    case CTRL_H:
    case DEL_KEY:
        deleteCharacter();
        break;
    case PAGE_UP:
    case PAGE_DOWN:
        if (s == PAGE_UP && quark.cy != 0)
            quark.cy = 0;
        else if (s == PAGE_DOWN && quark.cy != quark.screenrows-1)
            quark.cy = quark.screenrows-1;
        {
        int times = quark.screenrows;
        while(times--)
            quarkMoveCursor(s == PAGE_UP ? ARROW_UP:
                                            ARROW_DOWN);
        }
        break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        quarkMoveCursor(s);
        break;
    case CTRL_L:
        break;
    case ESC:
        break;
    default:
        insertCharacter(s);
        break;
    }
}

/* Framework for Raw Mode functionality. ------------------------------------*/

/* 
Exit raw mode to change editing framework. We need to enter the raw
mode for real-time processing.
Inputs:
	int f :  File number open for given process.
Outputs:	
    VOID.
*/
void exitRawMode(int f) {
    if (quark.rawmode) {
        tcsetattr(f,TCSAFLUSH,&orig_termios);
        quark.rawmode = 0;
    }
}

/* 
Disables raw mode at exit - input and return are VOID, requires 
constant from current session. 
*/
void editorAtExit(void) {
    exitRawMode(STDIN_FILENO);
}

/*
Enable raw mode to change editing framework. 
Inputs: 	
    int f:    File number open for given process.
Outputs:
    0  : If raw mode successfully enabled.
    -1 : If error condition is met.
*/
int enterRawMode(int f) {
	// Double check that we are not already in raw mode.
    if (quark.rawmode) return 0;

	// Start new process.
    struct termios raw;

    if (!isatty(STDIN_FILENO)) {
	    errno = ENOTTY;
    	return -1;
	}
	atexit(editorAtExit);
    if (tcgetattr(f,&orig_termios) == -1) {
		errno = ENOTTY;
		return -1;
	}
    raw = orig_termios; 
	// Turn off lots of flags and set a timeout.
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0; 
    raw.c_cc[VTIME] = 1;
	// Actually set the value to raw mode. 
    if (tcsetattr(f,TCSAFLUSH,&raw) < 0) {
		errno = ENOTTY;
		return -1;
	}
    quark.rawmode = 1;
    return 0;
}

/* Initialization. ----------------------------------------------------------*/

/* 
Initalize the object for the current session, make sure to get the screen set up.
Input:
    void
Output:
    void
*/
void initialize(void) {
    quark.cx = 0;
    quark.cy = 0;
    quark.rowoff = 0;
    quark.coloff = 0;
    quark.numrows = 0;
    quark.row = NULL;
    quark.unsaved = 0;
    quark.filename = NULL;
    quark.syntax = NULL;
    if (getWindowSize(STDIN_FILENO,STDOUT_FILENO,
                      &quark.screenrows,&quark.screencols) == -1)
    {
        perror("Screen failed to load. Sad");
        exit(1);
    }
	/* It might be nice to share the status or something... */
    quark.screenrows -= 2; 
}

/* Initializes the editor and associated status messages.*/ 
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,"Usage: quark <filename>\n");
        exit(1);
    }
	/* Initialize quark structure for current session, set up syntax
	   highlighting, and include a help status message. */
    initialize();
    determineHighlight(argv[1]);
    openFile(argv[1]);
    enterRawMode(STDIN_FILENO);
    setStatus("Ctrl + | S = save | Q = quit");
	/* Keep processing until method completes. */
    while(1) {
        editorRefreshScreen();
        editorKeyProcess(STDIN_FILENO);
    }
    return 0;
}
