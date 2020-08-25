#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zip.h>
#include <libxml/parser.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

#include "util.h"

#define LEN(a)		sizeof(a) / sizeof(a[0]) 
#define TEMPDIR	"/tmp/pptxt/" 

void writetofile(char *out_file_path, unsigned char *data, int n);
// int readzip(const char *path, char *filename, char *ext);
void readzip(const char *path);
void parseslidetext(const char *path, FILE *outfile);
void usage();
void cleanup();


void
writetofile(char *out_file_path, unsigned char *data, int n)
{
	FILE *out_file;
	out_file = fopen_mkdir(out_file_path, "wb");
	// ofprintf(out_file, "%u", data);
	fwrite(data, sizeof(data[0]), n, out_file);
	fclose(out_file);
	return;
}

void
readzip(const char *path)
{
	struct zip *zf;
	//struct zip_file *file;
	int err = 0;
	unsigned char *data;
	// char *tmpfilename;
	int i = 0;
	int num_entries;
//	char *buffer;
	char errorstr[100];

	if ((zf = zip_open(path, 0, &err)) == NULL) {
		zip_error_to_str(errorstr, sizeof(errorstr), err, errno);
		die("Unable to extract zip: %s: %s", path, errorstr);
	}


	num_entries = zip_get_num_entries(zf, ZIP_FL_UNCHANGED);
	for (i = 0; i < num_entries; i++) {
			const char *filename = zip_get_name(zf, i, ZIP_FL_ENC_GUESS);
			struct zip_stat st;
			zip_stat_init(&st);
		  	zip_stat(zf, filename, 0, &st);
			zip_file_t *file = zip_fopen(zf, filename, ZIP_FL_UNCHANGED);

			if (file == NULL) {
				fprintf(stderr, "couldnt extract file\n");
				printf("coudlnt extarnct fiel \n");
				zip_fclose(file);
				continue;
			}

			// data = ecalloc(sizeof(unsigned char), st.size + 10);
			data = xmalloc(sizeof(unsigned char) * st.size + 10);
			err = zip_fread(file, data, st.size);
			zip_fclose(file);
			if (err <= 0) {
				printf("coudlnt read filei\n");
				free(data);
				continue;
			}
			data[st.size] = '\0';

			char tmpfilename[sizeof(TEMPDIR) + sizeof(st.name)];
			sprintf(tmpfilename, "%s%s", TEMPDIR, st.name);

			writetofile(tmpfilename, data, st.size);

			free(data);
//			printf("%d / %d\n", i, num_entries - 1);
	}

	printf("closing zip file\n");
	err = zip_close(zf);

   if (err != 0) {
   die("Unable to close zip: %s", path);
   }
	printf("done\n");

	return;
}

void
parseslidetext(const char *path, FILE *outfile) 
{
	// xmlDoc *document;
	xmlDocPtr document;
	xmlNode *root;
	xmlNode *node[7];
	xmlChar *text;
	
	if (access(path, R_OK) == -1) {
		return;
	}

	document = xmlReadFile(path, NULL, 0);
	if (document == NULL) {
		return;
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
	fprintf(outfile, "\n");

	xmlFreeDoc(document);
	xmlCleanupParser();
	return;
}


void
parseslideimages(const char *path, FILE *outfile) 
{
	// xmlDoc *document;
	xmlDocPtr document;
	xmlNode *root;
	xmlNode *node;
	xmlChar *text;
	
	if (access(path, R_OK) == -1) {
		return;
	}

	document = xmlReadFile(path, NULL, 0);
	if (document == NULL) {
		return;
	}

	root = xmlDocGetRootElement(document);

	if (root == NULL) {
		die("No root in xml file");
	}

	if (!xmlStrEqual(root->name, (const xmlChar *) "Relationships")) {
		die("wrong xml format");
	}


	for (node = root->children; node; node = node->next) {
		if (xmlStrEqual(node->name, (const xmlChar *) "Relationship")) {
			text = xmlGetProp(node, (const xmlChar *) "Target");
			if (strstr((const char *) text, "media")) {
				fprintf(outfile, "\n@/tmp/pptxt/ppt/slides/%s\n", text);
				// printf("Target: %s\n", text);
			}
			xmlFree(text);
		}
	}
	fprintf(outfile, "\n");

	xmlFreeDoc(document);
	xmlCleanupParser();
	return;
}


void
cleanup()
{
	rmrf(TEMPDIR);
}

void
usage()
{
	die("usage: pptxt infile [-o outfile] [-c / --clean]");
	return;
}

int
main(int argc, char *argv[])
{
	FILE *outfile = NULL;
	char *outfilename = "out.txt";
	char *infilename = "";
	int i = 1;
	char *tmpfilename;

	if (argc < 2) {
		usage();
	}
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			printf("pptxt-0.1\n");
			return 0;
		} else if (!strcmp(argv[i], "-o")) {
			if (argc <= i - 1) {
				usage();
			}
			outfilename = argv[i + 1];
			i++;
		} else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--clean")) {
			printf("cleaning up\n");
			cleanup();
			return 0;
		} else {
			if (i == 1) {
				infilename = argv[1];
			} else {
				usage();
			}
		}
	}

	safe_create_dir(TEMPDIR);
	readzip(infilename);
	// printf("finished extracting zip\n");
	outfile = fopen(outfilename, "wt");

	while (1) {
		tmpfilename = xmalloc(sizeof("/tmp/pptxt/ppt/slides/slide") + (int)((ceil(log10(i))+1)*sizeof(char)) + sizeof(".xml"));

		sprintf(tmpfilename, "/tmp/pptxt/ppt/slides/slide%d.xml", i);

		if (access(tmpfilename, R_OK) == -1) {
			break;
		}

		fprintf(outfile, "# Slide %d\n", i);
		parseslidetext(tmpfilename, outfile);
		tmpfilename = xrealloc(tmpfilename, sizeof("/tmp/pptxt/ppt/slides/_rels/slide") + (int)((ceil(log10(i))+1)*sizeof(char)) + sizeof(".xml.rels"));
		sprintf(tmpfilename, "/tmp/pptxt/ppt/slides/_rels/slide%d.xml.rels", i);
		parseslideimages(tmpfilename, outfile);

		free(tmpfilename);
		i++;
	}
	fclose(outfile);

	return 0;

}
