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

void writetofile(char *out_file_path, char *data);
int readzip(const char *path, char *filename);
void parsexml(const char *path, FILE *outfile);
void usage();
void cleanup();


void
writetofile(char *out_file_path, char *data)
{
	FILE *out_file;
	out_file = fopen_mkdir(out_file_path, "w");
	fprintf(out_file, "%s", data);
	fclose(out_file);
	return;
}

int
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

	while (1) {
		tmpfilename = xmalloc(sizeof(filename) + (int)((ceil(log10(i))+1)*sizeof(char)) + sizeof(".xml"));
		buffer = xmalloc((int)((ceil(log10(i))+1)*sizeof(char)));
		strcpy(tmpfilename, filename);

		sprintf(buffer, "%d", i);
		strcat(tmpfilename, buffer);
		strcat(tmpfilename, ".xml");
		//strcat(tmpfilename, i);
		file = zip_fopen(zf, tmpfilename, ZIP_FL_UNCHANGED);

		if (file == NULL) {
			free(tmpfilename);
			free(buffer);
			break;
		}

		
		zip_stat(zf, tmpfilename, 0, &st);
		size = st.size;

		data = ecalloc(sizeof(char), size + 10);

		zip_fread(file, data, size);

		zip_fclose(file);

		char outfile[sizeof(TEMPFILE) + sizeof(tmpfilename)];
		sprintf(outfile, "%s%s", TEMPFILE, tmpfilename);

		data[size] = '\0';
		writetofile(outfile, data);

		free(data);
		free(tmpfilename);
		i++;
		free(buffer);
	}

	// zip_discard(zf);
	err = zip_close(zf);

	if (err != 0) {
		die("Unable to close zip: %s", path);
	}

	return i - 1;
}

void
parsexml(const char *path, FILE *outfile) 
{
	// xmlDoc *document;
	xmlDocPtr document;
	xmlNode *root;
	xmlNode *node[7];
	xmlChar *text;

	document = xmlReadFile(path, NULL, 0);
	if (document == NULL) {
		die("Unable to read xml file");
	}

	root = xmlDocGetRootElement(document);

	if (root == NULL) {
		die("No root in xml file");
	}

	if (!xmlStrEqual(root->name, (const xmlChar *) "sld")) {
		die("wrong xml format");
	}
	for (node[0] = root->children; node[0]; node[0] = node[0]->next) {
		if (xmlStrEqual(node[0]->name, (const xmlChar *) "cSld")) {
			for (node[1] = node[0]->children; node[1]; node[1] = node[1]->next) {
				if (xmlStrEqual(node[1]->name, (const xmlChar *) "spTree")) {
					for (node[2] = node[1]->children; node[2]; node[2] = node[2]->next) {
						if (xmlStrEqual(node[2]->name, (const xmlChar *) "sp")) {
							for (node[3] = node[2]->children; node[3]; node[3] = node[3]->next) {
								if (xmlStrEqual(node[3]->name, (const xmlChar *) "txBody")){
									for (node[4] = node[3]->children; node[4]; node[4] = node[4]->next) {
										if (xmlStrEqual(node[4]->name, (const xmlChar *) "p")){
											for (node[5] = node[4]->children; node[5]; node[5] = node[5]->next) {
												if (xmlStrEqual(node[5]->name, (const xmlChar *) "r")){
													for (node[6] = node[5]->children; node[6]; node[6] = node[6]->next){
														if (xmlStrEqual(node[6]->name, (const xmlChar *) "t")) {
															text = xmlNodeGetContent(node[6]);
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

							}
						}
					}
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
cleanup()
{
	rmrf(TEMPFILE);
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
	int n;
	char *tmpfilename;

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
	n = readzip(infilename, "ppt/slides/slide");
	// printf("finished extracting zip\n");
	outfile = fopen(outfilename, "wt");

	for (int i = 1; i <= n; i++) {
		tmpfilename = xmalloc(sizeof("/tmp/pptxt/ppt/slides/slide") + (int)((ceil(log10(i))+1)*sizeof(char)) + sizeof(".xml"));

		sprintf(tmpfilename, "/tmp/pptxt/ppt/slides/slide%d.xml", i);

		parsexml(tmpfilename, outfile);
		free(tmpfilename);
	}
	fclose(outfile);

	cleanup();


	return 0;

}
