/* Easter Egg! Opens relevant urls necessary for personal enrichment.
*  Thanks to Allen Downey for writing a variety of books.
*/

#include "eggs.h"

/* Function to read in input.
*/
void readin(char* string, int length) {
    int x;

    fgets(string,length,stdin);
    for(x=0;x<=length;x++)
    {
        if (string[x] == '\n')
        {
            string[x] = '\0';
            break;
        }
    }
}

/* Sets the url based on the input string.
*  Opens up relevant Allen Downey book, or if nothing matches,
*   opens up Allen Downey's website.
*/ 
void choose(char *input, char *url) {
	if (strstr(input, "operating systems") || strstr(input, "os")) {
		strcat(url, THINKOS);
	}
	else if (strstr(input, "perl")) {
		strcat(url, THINKPERL);
	}
	else if (strstr(input, "stat")) {
		strcat(url, THINKSTATS);
	}
	else if (strstr(input, "bayes")) {
		strcat(url, THINKBAYES);
	}
	else if (strstr(input, "dsp") || strstr(input, "digital signal processing")) {
		strcat(url, THINKDSP);
	}
	else if (strstr(input, "java")) {
		strcat(url, THINKJAVA);
	}
	else if (strstr(input, "ds") || strstr(input, "dast") || strstr(input, "data structures")) {
		strcat(url, THINKDS);
	}
	else if (strstr(input, "complexity")) {
		strcat(url, THINKCOM);
	}
	else if (strstr(input, "py")) {
		strcat(url, THINKPY);
	}
	else {
		strcat(url, ALLEN);
	}
}