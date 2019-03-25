/* Template for the Think easter eggs. Opens the URL of varied Allen books.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THINKOS "http://greenteapress.com/thinkos/thinkos.pdf"
#define THINKPERL "http://greenteapress.com/wp/think-perl-6/"
#define THINKSTATS "http://greenteapress.com/thinkstats2/thinkstats2.pdf"
#define THINKBAYES "http://www.greenteapress.com/thinkbayes/thinkbayes.pdf"
#define THINKDSP "http://greenteapress.com/thinkdsp/thinkdsp.pdf"
#define THINKJAVA "http://greenteapress.com/thinkjava6/thinkjava.pdf"
#define THINKDS "http://greenteapress.com/thinkdast/thinkdast.pdf"
#define THINKCOM "http://greenteapress.com/complexity2/thinkcomplexity2.pdf"
#define THINKPY "http://greenteapress.com/thinkpython2/thinkpython2.pdf"
#define ALLEN "http://www.allendowney.com/wp/"

void read(char* string, int length)
{
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

void choose (char *input, char *url) {
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

int main() {
	char *input = malloc(2048);
	char *url = malloc(60 + sizeof(char) * strlen("xdg-open "));

	read(input, 2048);

	if (strcmp(input, "teach me") == 0) {
		free(input);
		input = malloc(2048);
		read(input, 2048);
		strcpy(url, "xdg-open ");
		choose(input, url);
		system(url);
	}

	// printf("%s\n", url);
	return 0;

}
