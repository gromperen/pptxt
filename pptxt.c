#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zip.h>
#include <libxml/parser.h>
#include <math.h>
#include <errno.h>

#include "util.h"

#define LEN(a)		sizeof(a) / sizeof(a[0]) 
#define TEMPFILE	"/tmp/pptxt/" 

void
writetofile(char *out_file_path, char *data)
{
	FILE *out_file;
	out_file = fopen_mkdir(out_file_path, "w");
	fprintf(out_file,"%s", data);
	fclose(out_file);
	return;
}

void
readzip(const char *path, char *filename)
{
	struct zip *zf;
	zip_file_t *file;
	struct zip_stat st;
	int err = 0;
	int size;
	char *data;
	char *tmpfilename;
	int i = 1;
	char *buffer;
	char errorstr[100];

	if ((zf = zip_open(path, 0, &err)) == NULL) {
		zip_error_to_str(errorstr, sizeof(errorstr), err, errno);
		die("Unable to extract zip: %s: %s", path, errorstr);
	}
	/*
	zf = zip_open(path, ZIP_RDONLY, &err);

	if (err != 0) {
		die("Unable to extract zip: %s", path);
	}
	*/

	while (1) {
		tmpfilename = xmalloc(sizeof(filename) + (int)((ceil(log10(i))+1)*sizeof(char)) + sizeof(".xml"));
		buffer = xmalloc((int)((ceil(log10(i))+1)*sizeof(char)));
		strcpy(tmpfilename, filename);

		sprintf(buffer, "%d", i);
		strcat(tmpfilename, buffer);
		strcat(tmpfilename, ".xml");
		//strcat(tmpfilename, i);
		printf("%s\n",tmpfilename);
		file = zip_fopen(zf, tmpfilename, ZIP_FL_UNCHANGED);

		if (file == NULL) {
			printf("stopped at %s\n", tmpfilename);
			free(tmpfilename);
			printf("freed tmpfilename\n");
			free(buffer);
			printf("freed buffer\n");
			break;
		}

		
		zip_stat(zf, tmpfilename, 0, &st);
		size = st.size;

		data = ecalloc(sizeof(char), size);

		zip_fread(file, data, size);

		zip_fclose(file);

		char outfile[sizeof(TEMPFILE) + sizeof(tmpfilename)];
		sprintf(outfile, "%s%s", TEMPFILE, tmpfilename);
		printf("writing to %s\n", outfile);

		writetofile(outfile, data);
		free(data);
		free(tmpfilename);
		i++;
		free(buffer);
	}

	printf("closing zip\n");
	// zip_discard(zf);
	 err = zip_close(zf);

	if (err != 0) {
		die("Unable to close zip: %s", path);
	}

	return;
}

void
parsexml(const char *path, FILE *outfile) 
{
	// xmlDoc *document;
	xmlDocPtr document;
	xmlNode *root, *node_body, *node_p, *node_r, *node_t;
	xmlChar *text;

	document = xmlReadFile(path, NULL, 0);
	if (document == NULL) {
		die("Unable to read xml file");
	}

	root = xmlDocGetRootElement(document);

	if (root == NULL) {
		die("No root in xml file");
	}

	if (!xmlStrEqual(root->name, (const xmlChar *) "document")) {
		die("wrong xml format");
	}
	for (node_body = root->children; node_body; node_body = node_body->next) {
		if (xmlStrEqual(node_body->name, (const xmlChar *) "body")) {
			for (node_p = node_body->children; node_p; node_p = node_p->next) {
				if (xmlStrEqual(node_p->name, (const xmlChar *) "p")) {
					for (node_r = node_p->children; node_r; node_r = node_r->next) {
						if (xmlStrEqual(node_r->name, (const xmlChar *) "r")) {
							for (node_t = node_r->children; node_t; node_t = node_t->next) {
								if (xmlStrEqual(node_t->name, (const xmlChar *) "t")) {
									text = xmlNodeGetContent(node_t);
									fprintf(outfile, "%s", text);
									xmlFree(text);
								}
							}
						}
					}
					fprintf(outfile, "\n");
				}
			}
		}
		break;
	}

	xmlFreeDoc(document);
	xmlCleanupParser();
	return;
}

void
usage()
{
	die("usage: pptxt infile [-o outfile]");
	return;
}

int
main(int argc, char *argv[])
{
	FILE *outfile = NULL;
	char *outfilename = "out.txt";
	char *infilename = "";

	if (argc < 2) {
		usage();
	}
	infilename = argv[1];
	for (int i = 2; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			puts("doctxt-"VERSION);
			return 0;
		} else if (!strcmp(argv[i], "-o")) {
			if (argc <= i - 1) {
				usage();
			}
			outfilename = argv[i + 1];
			i++;
		}
		else {
			usage();
		}
	}

	safe_create_dir(TEMPFILE);
	readzip(infilename, "ppt/slides/slide");
	printf("finished extracting zip\n");
	outfile = fopen(outfilename, "wt");

	parsexml(TEMPFILE, outfile);
	fclose(outfile);

	if (remove(TEMPFILE) != 0) {
		die("Unable to delete tempfile");
	}

	return 0;

}
